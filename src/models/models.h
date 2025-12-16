#ifndef MODELS_H
#define MODELS_H

#include "glib.h"
#include "gtk/gtk.h"
#include <memory>

// defs
class AppState;
class HomeInstance;

// state
class AppState {
private:
  GtkApplication *app;
  GtkWidget *media_controls;
  GtkWidget *lyrics_label;
  GtkMediaStream *media_stream;
  float *audio_data;
  int data_size;
  char *filename;
  char **argv;
  int argc;
  std::unique_ptr<HomeInstance> home;
  void on_activate();
  static void on_activate_callback(GtkApplication *app, gpointer user_data);
  void load_actions();
  void load_views();

public:
  AppState(GtkApplication *app, char **argv, int argc);
  ~AppState();
  int run();
  GtkApplication *get_app();

  void set_current_filename(gchar *);
  gchar *get_current_filename();
};

// instances
class HomeInstance {
private:
  AppState *app_state;
  GtkWindow *win;
  GtkDrawingArea *drawing_area;
  guint timeout_id;
  int position;

  void draw_stand_by_function(cairo_t *cr, int width, int height);
  static gboolean on_timeout(gpointer user_data);

public:
  HomeInstance(AppState *state);
  ~HomeInstance();
  int show();
};

#define MODELS_H
#endif // !MODELS_H
