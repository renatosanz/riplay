#include "glib.h"
#include "glibconfig.h"
#include "glibmm/main.h"
#include "glibmm/refptr.h"
#include "glibmm/ustring.h"
#include "gtkmm/label.h"
#include "gtkmm/mediastream.h"
#include "metadata/metadata.h"
#include "sigc++/functors/mem_fun.h"
#include "tstringlist.h"
#include "types.h"
#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <gtk/gtk.h>
#include <iostream>
#include <ostream>
#include <regex>
#include <stdexcept> // Para manejo de errores
#include <string>
#include <taglib/attachedpictureframe.h>
#include <taglib/fileref.h>
#include <taglib/flacfile.h>
#include <taglib/id3v2frame.h>
#include <taglib/id3v2header.h>
#include <taglib/id3v2tag.h>
#include <taglib/mp4file.h>
#include <taglib/mpegfile.h>
#include <taglib/tag.h>
#include <taglib/tpropertymap.h>
#include <taglib/unsynchronizedlyricsframe.h>
#include <variant>
#include <vector>
#include <xiphcomment.h>

static std::vector<std::string> f_lrc_props = {"by", "offset"};

gchar *convert_to_utf8(const char *input) {
  char *utf8_str = g_locale_from_utf8(input, NULL, NULL, NULL, NULL);
  if (!utf8_str) {
    utf8_str = g_strdup(input);
  }
  return utf8_str;
}

LyricsManager::LyricsManager(std::shared_ptr<SongInstance> song) {
  auto extracted_lyrics_data = extractLyrics(song->get_filepath());

  if (std::holds_alternative<std::string>(extracted_lyrics_data)) {
    raw_lyrics = std::get<std::string>(extracted_lyrics_data);
    lyric_props = std::vector<LyricProp>({});
    sync_lyrics = parser_lyrics(raw_lyrics, lyric_props);
  }
}

int LyricsManager::update_lyric() {
  if (!stream || !stream->get_playing()) {
    return false;
  }

  int64_t current_time = stream->get_timestamp(); // Convert to ms
  // printf("updating lyrics in %ld \n", current_time);
  for (size_t i = lyrics_index; i < sync_lyrics.size(); i++) {
    if (current_time >= sync_lyrics[i].timestamp) {
      if (i != lyrics_index) {
        lyrics_index = i;
        lyrics_label->set_label(sync_lyrics[i].lyric);
      }
    } else {
      break;
    }
  }

  return true;
}

void LyricsManager::start_lyrics_display(Glib::RefPtr<Gtk::MediaStream> stream,
                                         Glib::RefPtr<Gtk::Label> label) {
  if (sync_lyrics.empty() || !stream) {
    return;
  }
  lyrics_label = label;
  this->stream = stream;

  if (!sync_lyrics.empty()) {
    lyrics_label->set_label(sync_lyrics[0].lyric.c_str());
    lyrics_index = 0;
    lyric_sync_connection = Glib::signal_timeout().connect(
        sigc::mem_fun(*this, &LyricsManager::update_lyric),
        240); // check every 100ms

    if (!lyric_props.empty()) {
      for (auto x : lyric_props) {
        std::cout << x.field << " : " << x.value << std::endl;
      }
    }
  } else if (raw_lyrics.length() > 0) {
    lyrics_label->set_label(raw_lyrics);
  }
}

void LyricsManager::stop_synced_lyrics() {
  if (lyric_sync_connection.connected()) {
    lyric_sync_connection.disconnect();
  }
}

std::variant<bool, std::string>
LyricsManager::extractLyrics(std::string filePath) {
  TagLib::FileRef file(filePath.c_str());

  if (!file.isNull() && file.tag()) {
    // handle mp3 files (id3v2 tags)
    if (TagLib::MPEG::File *mpegFile =
            dynamic_cast<TagLib::MPEG::File *>(file.file())) {
      if (mpegFile->ID3v2Tag()) {
        // try both uslt (unsynchronized lyrics) and sylt (synchronized lyrics)
        // frames
        TagLib::ID3v2::FrameList lyricsFrames =
            mpegFile->ID3v2Tag()->frameList("USLT");
        if (lyricsFrames.isEmpty()) {
          lyricsFrames = mpegFile->ID3v2Tag()->frameList("SYLT");
        }

        if (!lyricsFrames.isEmpty()) {
          TagLib::ID3v2::UnsynchronizedLyricsFrame *lyricsFrame =
              static_cast<TagLib::ID3v2::UnsynchronizedLyricsFrame *>(
                  lyricsFrames.front());
          return lyricsFrame->toStringList().toString("\n").to8Bit(true);
        }
      }
    }
    // handle flac files (usually in vorbis comments)
    else if (TagLib::FLAC::File *flacFile =
                 dynamic_cast<TagLib::FLAC::File *>(file.file())) {
      if (flacFile->xiphComment()) {
        TagLib::Ogg::XiphComment *xiphComment = flacFile->xiphComment();
        if (xiphComment->contains("LYRICS")) {
          return xiphComment->fieldListMap()["LYRICS"].toString("\n").to8Bit(
              true);
        }
        // Some files might use different field names
        else if (xiphComment->contains("LYRICS")) {
          return xiphComment->fieldListMap()["LYRICS"].toString("\n").to8Bit(
              true);
        }
      }
    }
    // handle aac/mp4 files
    else if (TagLib::MP4::File *mp4File =
                 dynamic_cast<TagLib::MP4::File *>(file.file())) {
      if (mp4File->tag()) {
        TagLib::MP4::Tag *tag = mp4File->tag();
        if (tag->contains("©lyr")) {
          return tag->item("©lyr").toStringList().toString("\n").to8Bit(true);
        }
      }
    }
  }

  return false;
}

// this function takes the raw lyric line, then parses the time of the lyric,
// and finally saves the timestamp in micronseconds with the lyric text in a
// LyricBar variable, lines that don't matches with the time lyric format
// [00:00.00] are returned as strings
std::variant<std::string, LyricProp, LyricBar>
format_lyric(const std::string &raw_lyric) {
  std::regex pattern(R"(^\[(\d{2}):(\d{2})\.(\d{2})\]\s(.+)$)");
  std::smatch matches;

  if (std::regex_match(raw_lyric, matches, pattern)) {
    int mins = std::stoi(matches[1].str());
    int secs = std::stoi(matches[2].str());
    int cents = std::stoi(matches[3].str());
    std::string lyric = matches[4].str();

    return LyricBar(
        {(guint64)((mins * 60000 + secs * 1000 + cents * 10) * 1000), lyric});
  } else {
    std::regex prop_pattern(R"(^\[(.+):(.+)])");
    std::smatch prop_matches;
    if (std::regex_match(raw_lyric, prop_matches, prop_pattern)) {
      std::string field = prop_matches[1].str();
      std::string value = prop_matches[2].str();
      // std::cout << field << ":" << value << std::endl;
      if (std::find(f_lrc_props.begin(), f_lrc_props.end(), field) !=
          f_lrc_props.end()) {
        return LyricProp({field, value});
      }
    } else {
      throw std::invalid_argument(
          "invalid lyric format, must be '[00:00.00] some lyrics...'");
    }
  }
  return "";
}

std::vector<std::string> split_to_array(const std::string &text) {
  std::vector<std::string> lines;
  size_t init = 0;
  size_t end = text.find('\n');

  while (end != std::string::npos) {
    lines.push_back(text.substr(init, end - init));
    init = end + 1;
    end = text.find('\n', init);
  }

  lines.push_back(text.substr(init));
  return lines;
}

std::vector<LyricBar> parser_lyrics(std::string lyrics,
                                    std::vector<LyricProp> &lyric_props) {
  std::vector<LyricBar> lyrics_formatted({});

  auto lines = split_to_array(lyrics);
  for (auto a : lines) {
    try {
      auto res = format_lyric(a);
      if (std::holds_alternative<LyricProp>(res)) {
        lyric_props.push_back(std::get<LyricProp>(res));
      } else if (std::holds_alternative<LyricBar>(res)) {
        lyrics_formatted.push_back(std::get<LyricBar>(res));
        // std::cout << std::get<LyricBar>(res_l).lyric << " - "
        //           << std::get<LyricBar>(res_l).timestamp << std::endl;
      }
    } catch (const std::exception &e) {
      std::cerr << "Error: " << e.what() << std::endl;
    }
  }

  return lyrics_formatted;
}
