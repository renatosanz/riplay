#ifndef METADATA_H
#define METADATA_H
#include "types.h"

FileMetaData *get_metadata(const char *filename);

unsigned char *extract_album_art(const char *filepath, unsigned long *size);
#endif
