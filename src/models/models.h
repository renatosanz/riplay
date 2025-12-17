#ifndef MODELS_H
#define MODELS_H

#include "glib.h"
#include "gtk/gtk.h"
#include "gtkmm/application.h"
#include "gtkmm/drawingarea.h"
#include "gtkmm/window.h"
#include <memory>

// defs
class AppState;
class HomeInstance;
class RecentsInstance;

// state
class AppState {
private:
  Glib::RefPtr<Gtk::Application> app;
  GtkWidget *media_controls;
  GtkWidget *lyrics_label;
  GtkMediaStream *media_stream;
  float *audio_data;
  int data_size;
  char *filename;
  char **argv;
  int argc;

  std::unique_ptr<HomeInstance> home;
  RecentsInstance *recents;

  void on_activate();
  static void on_activate_callback(GtkApplication *app, gpointer user_data);
  void load_actions();
  void load_views();

public:
  AppState(Glib::RefPtr<Gtk::Application>, char **argv, int argc);
  ~AppState();
  int run();
  Glib::RefPtr<Gtk::Application> get_app();

  void set_current_filename(gchar *);
  gchar *get_current_filename();
};

// instances
class HomeInstance {
private:
  AppState *state;
  Glib::RefPtr<Gtk::Window> win;
  Glib::RefPtr<Gtk::DrawingArea> drawing_area;
  sigc::connection timeout_id;
  int position;

  void draw_stand_by_function(const std::shared_ptr<Cairo::Context> &cr,
                              int width, int height);
  bool on_timeout();

public:
  HomeInstance(AppState *state);
  ~HomeInstance();
  void show();
};

class RecentsInstance {
private:
  AppState *state;
  GtkWindow *win;
  GtkBox *recent_files_box;

public:
  RecentsInstance(AppState *state);
  ~RecentsInstance();
  void show();
  static void lauch_by_action(GSimpleAction *_action, GVariant *_parameter,
                              gpointer user_data);
};

#define MODELS_H
#endif // !MODELS_H
