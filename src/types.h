#ifndef TYPES_H
#define TYPES_H

#include "glib.h"
#include "iostream"
#include <cairo.h>
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
  unsigned char *raw_albumart;
} FileMetadata;

typedef struct {
  guint64 timestamp;
  std::string lyric;
} LyricBar;

typedef struct {
  std::string field;
  std::string value;
} LyricProp;

typedef struct {
  GtkWindow *win;

  // animations
  guint timeout_id;
  GtkDrawingArea *drawing_area;
} HomeView;

typedef struct {
  GtkWindow *win;
  gchar *recent_file_selected;
} RecentsView;

typedef struct {
  GtkWindow *win;
  GtkWidget *media_controls;
  GtkWidget *lyrics_label;
} PlayerView;

typedef struct {
  GtkApplication *app;

  RecentsView *recents;
  PlayerView *player;

  GtkWidget *media_controls;
  GtkWidget *lyrics_label;
  GtkMediaStream *media_stream;
  float *audio_data;
  int data_size;
  char *filename;
  FileMetadata *metadata;
  GMutex data_mutex;
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

// instances

#endif
