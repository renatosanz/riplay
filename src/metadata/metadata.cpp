#include "metadata.h"
#include "types.h"
#include <cstring>
#include <glib.h>
#include <memory>
#include <taglib/audioproperties.h>
#include <taglib/fileref.h>
#include <taglib/id3v2tag.h>
#include <taglib/mpegfile.h>
#include <taglib/tag.h>
#include <taglib/tpropertymap.h>

std::shared_ptr<FileMetadata> extract_metadata_from_path(std::string filename) {
  // Use TagLib's C++ API directly
  TagLib::FileRef file(filename.c_str());

  if (file.isNull() || !file.tag()) {
    g_printerr("Error opening file or no tags found\n");
    return nullptr;
  }

  auto metadata = std::make_shared<FileMetadata>();

  // Get the tag information
  TagLib::Tag *tag = file.tag();
  if (tag) {
    strncpy(metadata->title, tag->title().toCString(true),
            sizeof(metadata->title) - 1);
    strncpy(metadata->artist, tag->artist().toCString(true),
            sizeof(metadata->artist) - 1);
    strncpy(metadata->album, tag->album().toCString(true),
            sizeof(metadata->album) - 1);
    metadata->year = tag->year();
    metadata->track = tag->track();
    strncpy(metadata->genre, tag->genre().toCString(true),
            sizeof(metadata->genre) - 1);
  }

  // Get audio properties
  if (file.audioProperties()) {
    const TagLib::AudioProperties *props = file.audioProperties();
    metadata->properties = g_new0(AudioProps, 1);
    if (metadata->properties) {
      metadata->properties->length = props->length();
      metadata->properties->bitrate = props->bitrate();
      metadata->properties->samplerate = props->sampleRate();
      metadata->properties->channels = props->channels();
    }
  }

  g_print("title: %s\n", metadata->title);

  metadata->raw_albumart =
      extractAlbumArt(filename.c_str(), &metadata->raw_albumart_size);

  return metadata;
}
