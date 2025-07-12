#ifndef TYPES_H 
#define TYPES_H

#include <cairo.h>
#include <gstreamer-1.0/gst/gst.h>
#include <gtk/gtk.h>

#define MAX_TEXT_DATA 128

typedef struct {
  int length;
  int samplerate;
  int channels;
  int bitrate;
}AudioProps ;

typedef struct {
  char title[MAX_TEXT_DATA];
  char artist[MAX_TEXT_DATA];
  char album[MAX_TEXT_DATA];
  char genre[MAX_TEXT_DATA];
  int year;
  int track;
  AudioProps *propieties;
} FileMetaData ;

typedef struct {
  GtkWidget *drawing_area;
  GtkWidget *media_controls;
  GtkWindow *win;
  GtkMediaStream *media_stream;
  float *audio_data;
  int data_size;
  guint timeout_id;
  char *filename;
  FileMetaData *metadata;
} AppData;

#endif
