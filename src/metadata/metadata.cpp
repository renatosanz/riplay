#include "metadata.h"
#include "types.h"
#include <glib.h>
#include <memory>
#include <string>
#include <taglib/audioproperties.h>
#include <taglib/fileref.h>
#include <taglib/id3v2tag.h>
#include <taglib/mpegfile.h>
#include <taglib/tag.h>
#include <taglib/tpropertymap.h>

namespace {
void copy_tag(const TagLib::String &value, char *dest, size_t size) {
  const std::string utf8 = value.to8Bit(true);
  g_strlcpy(dest, utf8.c_str(), size);
}
} // namespace

std::shared_ptr<FileMetadata> extract_metadata_from_path(std::string filename) {
  // Use TagLib's C++ API directly
  TagLib::FileRef file(filename.c_str());

  if (file.isNull() || !file.tag()) {
    g_printerr("Error opening file or no tags found\n");
    return nullptr;
  }

  auto metadata = std::make_shared<FileMetadata>();
  metadata->properties = new AudioProps{};

  // Get the tag information
  TagLib::Tag *tag = file.tag();
  if (tag) {
    copy_tag(tag->title(), metadata->title, sizeof(metadata->title));
    copy_tag(tag->artist(), metadata->artist, sizeof(metadata->artist));
    copy_tag(tag->album(), metadata->album, sizeof(metadata->album));
    copy_tag(tag->genre(), metadata->genre, sizeof(metadata->genre));
    metadata->year = tag->year();
    metadata->track = tag->track();
  }

  // Get audio properties
  if (file.audioProperties()) {
    const TagLib::AudioProperties *props = file.audioProperties();
    metadata->properties->length = props->length();
    metadata->properties->bitrate = props->bitrate();
    metadata->properties->samplerate = props->sampleRate();
    metadata->properties->channels = props->channels();
  }

  metadata->raw_albumart =
      extractAlbumArt(filename.c_str(), &metadata->raw_albumart_size);

  return metadata;
}
