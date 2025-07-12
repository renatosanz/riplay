#include "glib.h"
#include "types.h"
#include <metadata.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <taglib/tag_c.h>

FileMetaData *get_metadata(const char *filename) {
  FileMetaData *metadata = NULL;
  TagLib_File *file = taglib_file_new(filename);

  if (!file || !taglib_file_is_valid(file)) {
    fprintf(stderr, "Error al abrir el archivo\n");
    return NULL;
  }

  TagLib_Tag *tag = taglib_file_tag(file);

  if (tag) {
    metadata = malloc(sizeof(FileMetaData));
    if (!metadata)
      return NULL;
    strcpy(metadata->title,
           taglib_tag_title(tag) ? taglib_tag_title(tag) : "none");
    strcpy(metadata->artist,
           taglib_tag_artist(tag) ? taglib_tag_artist(tag) : "none");
    strcpy(metadata->album,
           taglib_tag_album(tag) ? taglib_tag_album(tag) : "none");
    metadata->year = taglib_tag_year(tag);
    metadata->track = taglib_tag_track(tag);
    strcpy(metadata->genre,
           taglib_tag_genre(tag) ? taglib_tag_genre(tag) : "none");
  }

  TagLib_AudioProperties *props = taglib_file_audioproperties(file);
  if (props) {
    metadata->propieties = malloc(sizeof(AudioProps));
    if (!metadata->propieties)
      return NULL;
    metadata->propieties->length = taglib_audioproperties_length(props);
    metadata->propieties->bitrate = taglib_audioproperties_bitrate(props);
    metadata->propieties->samplerate = taglib_audioproperties_samplerate(props);
    metadata->propieties->channels = taglib_audioproperties_channels(props);
  }

  g_print("title: %s", metadata->title);

  taglib_tag_free_strings();
  taglib_file_free(file);
  return metadata;
}
