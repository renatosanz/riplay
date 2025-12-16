#include "glib.h"
#include "glib/gprintf.h"
#include "glibconfig.h"
#include "tstringlist.h"
#include "types.h"
#include <algorithm>
#include <atomic>
#include <cstddef>
#include <cstdlib>
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
#include <thread>
#include <variant>
#include <vector>
#include <xiphcomment.h>

static std::atomic<bool> lyrics_running(false);
static std::thread lyrics_thread;
static std::vector<LyricBar> current_lyrics;
static GtkLabel *lyrics_label = nullptr;
static std::vector<std::string> f_lrc_props = {"by", "offset"};

gchar *convert_to_utf8(const char *input) {
  char *utf8_str = g_locale_from_utf8(input, NULL, NULL, NULL, NULL);
  if (!utf8_str) {
    utf8_str = g_strdup(input);
  }
  return utf8_str;
}

static gboolean update_lyric_label(gpointer user_data) {
  const char *text = (const char *)(user_data);
  // gtk_label_set_text(lyrics_label, (char *)convert_to_utf8(text));
  gtk_label_set_text(lyrics_label, text);
  return G_SOURCE_REMOVE; // eliminar el source después de ejecutar
}

static void lyrics_worker(GtkMediaStream *stream) {
  size_t index = 0;
  const size_t lyrics_count = current_lyrics.size();

  while (lyrics_running && index < lyrics_count) {
    if (gtk_media_stream_get_playing(stream)) {

      guint64 current_time = gtk_media_stream_get_timestamp(stream);

      if (current_lyrics[index].timestamp > current_time) {
        index = 0;
        for (auto x : current_lyrics) {
          if (x.timestamp < current_time) {
            index++;
          }
        }
        if (index > 0)
          index--;
      }
      while (index < lyrics_count &&
             current_lyrics[index].timestamp <= current_time) {
        g_idle_add(update_lyric_label,
                   (gpointer)current_lyrics[index].lyric.c_str());
        index++;
      }
    } else if (index == lyrics_count - 1) {
      index = 0;
      lyrics_running = true;
      g_printf("flaaaaaaaag!! index : %d  lyrics_count : %d", index,
               lyrics_count);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }
}

void stop_lyrics_display() {
  lyrics_running = false;

  if (lyrics_thread.joinable()) {
    lyrics_thread.join();
  }

  current_lyrics.clear();
  lyrics_label = nullptr;
}

void start_lyrics_display(const std::vector<LyricBar> &lyrics,
                          GtkMediaStream *stream, GtkLabel *label) {
  stop_lyrics_display();
  current_lyrics = lyrics;
  lyrics_label = label;
  lyrics_running = true;

  try {
    lyrics_thread = std::thread(lyrics_worker, stream);
  } catch (...) {
    lyrics_running = false;
    g_warning("No se pudo crear el hilo de lyrics");
  }
}

std::variant<bool, std::string> extractLyrics(const char *filePath) {
  TagLib::FileRef file(filePath);

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
