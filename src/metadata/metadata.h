#ifndef METADATA_H
#define METADATA_H
#include "types.h"
#include <memory>

std::shared_ptr<FileMetadata> extract_metadata_from_path(std::string filename);
unsigned char *extractAlbumArt(const char *filePath, unsigned long *size);
#endif
