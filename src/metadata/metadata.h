#ifndef METADATA_H
#define METADATA_H
#include "gtkmm/label.h"
#include "gtkmm/mediastream.h"
#include "types.h"
#include <memory>
#include <variant>

std::shared_ptr<FileMetadata> extract_metadata_from_path(std::string filename);
unsigned char *extractAlbumArt(const char *filePath, unsigned long *size);
std::variant<bool, std::string> extractLyrics(std::string filePath);
std::vector<LyricBar> parser_lyrics(std::string lyrics,
                                    std::vector<LyricProp> &lyric_props);
void start_lyrics_display(const std::vector<LyricBar> &lyrics,
                          Glib::RefPtr<Gtk::MediaStream> stream,
                          Glib::RefPtr<Gtk::Label> label);
void end_lyrics_thread();

#endif
