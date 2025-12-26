#ifndef METADATA_H
#define METADATA_H
#include "gtkmm/label.h"
#include "gtkmm/mediastream.h"
#include "models/models.h"
#include "types.h"
#include <atomic>
#include <memory>
#include <variant>

std::shared_ptr<FileMetadata> extract_metadata_from_path(std::string filename);
unsigned char *extractAlbumArt(const char *filePath, unsigned long *size);
// std::variant<bool, std::string> extractLyrics(std::string filePath);
std::vector<LyricBar> parser_lyrics(std::string lyrics,
                                    std::vector<LyricProp> &lyric_props);
void start_lyrics_display(const std::vector<LyricBar> &lyrics,
                          Glib::RefPtr<Gtk::MediaStream> stream,
                          Glib::RefPtr<Gtk::Label> label);
void end_lyrics_thread();

class LyricsManager {
private:
  std::atomic_bool is_sync;
  std::string raw_lyrics;             // plain text lyrics
  std::vector<LyricBar> sync_lyrics;  // timed synced lyrics
  std::vector<LyricProp> lyric_props; // synced lyrics props
  Glib::RefPtr<Gtk::MediaStream> stream;
  Glib::RefPtr<Gtk::Label> lyrics_label = nullptr;
  sigc::connection lyric_sync_connection;
  std::atomic<size_t> lyrics_index = 0;

public:
  LyricsManager(std::shared_ptr<SongInstance>);
  unsigned char *extractAlbumArt(unsigned long *size);
  std::variant<bool, std::string> extractLyrics(std::string filePath);
  int update_lyric();
  void start_lyrics_display(Glib::RefPtr<Gtk::MediaStream> stream,
                            Glib::RefPtr<Gtk::Label> label);
  void stop_synced_lyrics();
};

#endif
