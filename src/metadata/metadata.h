#ifndef METADATA_H
#define METADATA_H
#include "audio/audio_manager.h"
#include "gtkmm/label.h"
#include "models/models.h"
#include "types.h"
#include <atomic>
#include <memory>
#include <string>
#include <variant>
#include <vector>

std::shared_ptr<FileMetadata> extract_metadata_from_path(std::string filename);
unsigned char *extractAlbumArt(const char *filePath, unsigned long *size);
std::vector<LyricBar> parser_lyrics(std::string lyrics,
                                    std::vector<LyricProp> &lyric_props);

class LyricsManager {
private:
  std::atomic_bool is_sync{false};
  std::atomic_bool has_lyrics{false};
  std::string raw_lyrics;             // plain text lyrics
  std::vector<LyricBar> sync_lyrics;  // timed synced lyrics
  std::vector<LyricProp> lyric_props; // synced lyrics props
  AudioManager *audio = nullptr;      // non owned, lives in AppState
  Glib::RefPtr<Gtk::Label> lyrics_label = nullptr;
  sigc::connection lyric_sync_connection;
  std::atomic<size_t> lyrics_index = 0;

  void show_lyric(size_t index);

public:
  explicit LyricsManager(std::shared_ptr<SongInstance> song);
  std::variant<bool, std::string> extractLyrics(std::string filePath);
  bool update_lyric();
  void setup(AudioManager *audio_manager, Glib::RefPtr<Gtk::Label> label);
  void stop_synced_lyrics();
  void toggle_update_lyrics(bool is_visible);
  void continue_synced_lyrics();
};

#endif
