#include <cstring>
#include <glib.h>
#include <metadata.h>
#include <taglib/audioproperties.h>
#include <taglib/fileref.h>
#include <taglib/id3v2tag.h>
#include <taglib/mpegfile.h>
#include <taglib/tag.h>
#include <taglib/tpropertymap.h>

FileMetaData *get_metadata(const char *filename) {
  // Use TagLib's C++ API directly
  TagLib::FileRef file(filename);

  if (file.isNull() || !file.tag()) {
    g_printerr("Error opening file or no tags found\n");
    return nullptr;
  }

  FileMetaData *metadata = g_new0(FileMetaData, 1);
  if (!metadata) {
    return nullptr;
  }

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

  g_print("title: %s", metadata->title);

  return metadata;
}
