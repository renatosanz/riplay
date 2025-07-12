#ifndef TYPES_H 
#define TYPES_H

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

#endif
