#include "audio/audio_manager.h"
#include "glibmm/main.h"
#include "gtkmm/label.h"
#include "metadata/metadata.h"
#include "sigc++/functors/mem_fun.h"
#include "types.h"
#include <algorithm>
#include <cstdint>
#include <exception>
#include <iostream>
#include <regex>
#include <stdexcept>
#include <string>
#include <taglib/fileref.h>
#include <taglib/flacfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/mp4file.h>
#include <taglib/mpegfile.h>
#include <taglib/unsynchronizedlyricsframe.h>
#include <taglib/xiphcomment.h>
#include <variant>
#include <vector>

static std::vector<std::string> f_lrc_props = {"by", "offset"};

namespace {
const int64_t MICROSECONDS_PER_SECOND = 1000000;
const unsigned int SYNC_INTERVAL_MS = 240;
} // namespace

LyricsManager::LyricsManager(std::shared_ptr<SongInstance> song) {
  auto extracted_lyrics_data = extractLyrics(song->get_filepath());

  if (std::holds_alternative<std::string>(extracted_lyrics_data)) {
    raw_lyrics = std::get<std::string>(extracted_lyrics_data);
    lyric_props = std::vector<LyricProp>({});
    sync_lyrics = parser_lyrics(raw_lyrics, lyric_props);
  }
}

void LyricsManager::setup(AudioManager *audio_manager,
                          Glib::RefPtr<Gtk::Label> label) {
  if (!audio_manager || !label) {
    return;
  }

  audio = audio_manager;
  lyrics_label = label;

  is_sync = !sync_lyrics.empty();
  has_lyrics = !raw_lyrics.empty();

  if (is_sync) {
    show_lyric(lyrics_index);
  } else if (has_lyrics) {
    lyrics_label->set_label(raw_lyrics);
  }
}

void LyricsManager::show_lyric(size_t index) {
  if (!lyrics_label || index >= sync_lyrics.size()) {
    return;
  }

  lyrics_index = index;
  lyrics_label->set_label(sync_lyrics[index].lyric);
}

bool LyricsManager::update_lyric() {
  if (!audio || !is_sync || !lyrics_label) {
    return false;
  }

  if (audio->getState() != PlaybackState::PLAYING) {
    if (audio->getState() == PlaybackState::STOPPED) {
      show_lyric(0);
    }
    return true;
  }

  uint64_t current_time =
      static_cast<uint64_t>(audio->getPositionSeconds()) *
      MICROSECONDS_PER_SECOND;

  for (size_t i = lyrics_index; i < sync_lyrics.size(); i++) {
    if (current_time < sync_lyrics[i].timestamp) {
      break;
    }
    if (i != lyrics_index) {
      show_lyric(i);
    }
  }

  return true;
}

void LyricsManager::toggle_update_lyrics(bool is_visible) {
  if (!is_visible) {
    stop_synced_lyrics();
  } else if (is_sync) {
    continue_synced_lyrics();
  }
}

void LyricsManager::continue_synced_lyrics() {
  if (lyric_sync_connection.connected()) {
    return;
  }

  lyric_sync_connection = Glib::signal_timeout().connect(
      sigc::mem_fun(*this, &LyricsManager::update_lyric), SYNC_INTERVAL_MS);
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
  }

  std::regex prop_pattern(R"(^\[(.+):(.+)])");
  std::smatch prop_matches;
  if (std::regex_match(raw_lyric, prop_matches, prop_pattern)) {
    std::string field = prop_matches[1].str();
    std::string value = prop_matches[2].str();
    if (std::find(f_lrc_props.begin(), f_lrc_props.end(), field) !=
        f_lrc_props.end()) {
      return LyricProp({field, value});
    }
  } else {
    throw std::invalid_argument(
        "invalid lyric format, must be '[00:00.00] some lyrics...'");
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
  for (auto line : lines) {
    try {
      auto res = format_lyric(line);
      if (std::holds_alternative<LyricProp>(res)) {
        lyric_props.push_back(std::get<LyricProp>(res));
      } else if (std::holds_alternative<LyricBar>(res)) {
        lyrics_formatted.push_back(std::get<LyricBar>(res));
      }
    } catch (const std::exception &e) {
      std::cerr << "Error: " << e.what() << std::endl;
    }
  }

  return lyrics_formatted;
}
