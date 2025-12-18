#ifndef METADATA_H
#define METADATA_H
#include "tstringlist.h"
#include "types.h"
#include "variant"

FileMetadata *get_metadata(const char *filename);
unsigned char *extractAlbumArt(const char *filePath, unsigned long *size);
unsigned char *extract_album_art(const char *filepath, unsigned long *size);
void start_lyrics_display(const std::vector<LyricBar> &lyrics,
                          GtkMediaStream *stream, GtkLabel *lyrics_label);
std::vector<LyricBar> parser_lyrics(std::string lyrics,
                                    std::vector<LyricProp> &lyrics_props);
std::variant<bool, std::string> extractLyrics(const char *filePath);
void stop_lyrics_display();
#endif
