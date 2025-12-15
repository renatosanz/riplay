#ifndef TYPES_H
#define TYPES_H

#include "iostream"
#include <cairo.h>
#include <gstreamer-1.0/gst/gst.h>
#include <gtk/gtk.h>
#include <vector>

#define MAX_TEXT_DATA 128

typedef struct {
  int length;
  int samplerate;
  int channels;
  int bitrate;
} AudioProps;

typedef struct {
  char title[MAX_TEXT_DATA];
  char artist[MAX_TEXT_DATA];
  char album[MAX_TEXT_DATA];
  char genre[MAX_TEXT_DATA];
  int year;
  int track;
  AudioProps *properties;
} FileMetaData;

typedef struct {
  guint64 timestamp;
  std::string lyric;
} LyricBar;

typedef struct {
  std::string field;
  std::string value;
} LyricProp;

typedef struct {
  GtkWidget *drawing_area;
  GtkWidget *media_controls;
  GtkWidget *lyrics_label;
  GtkWindow *win;
  GtkMediaStream *media_stream;
  float *audio_data;
  int data_size;
  guint timeout_id;
  char *filename;
  FileMetaData *metadata;
  GMutex data_mutex;
  GstElement *pipeline;
  std::vector<LyricBar> lyrics;
  std::vector<LyricProp> lyric_props;
} AppData;

typedef struct {
  GtkScale *band_scale;
  GtkLabel *band_label;
  GtkBox *band_cont;
  gfloat max_value;
  gfloat min_value;
  gfloat default_value;
  gfloat value;
  gfloat frecuency;
} EqualizerSliderData;

#endif
