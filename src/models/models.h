#ifndef MODELS_H
#define MODELS_H

#include "glib.h"
#include "gtk/gtk.h"
#include "gtkmm/application.h"
#include "gtkmm/drawingarea.h"
#include "gtkmm/label.h"
#include "gtkmm/widget.h"
#include "gtkmm/window.h"
#include "types.h"
#include <gtkmm/box.h>
#include <memory>

// defs
class AppState;
class HomeInstance;
class RecentsInstance;
class PlayerInstance;
class SongInstance;

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

  std::shared_ptr<SongInstance> current_song;

  std::unique_ptr<HomeInstance> home;
  std::unique_ptr<RecentsInstance> recents;
  std::unique_ptr<PlayerInstance> player;

  void on_activate();
  void load_actions();
  void load_views();

public:
  AppState(Glib::RefPtr<Gtk::Application>, char **argv, int argc);
  ~AppState();
  int run();
  Glib::RefPtr<Gtk::Application> get_app();
  std::shared_ptr<SongInstance> get_song();
  void open_player(Glib::ustring filepath);

  void set_current_filename(gchar *);
  gchar *get_current_filename();
};

class SongInstance {

private:
  std::string filepath;
  std::shared_ptr<FileMetadata> metadata;

public:
  SongInstance(std::string filepath);
  std::string get_filepath();
  // ~SongInstance();
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
  void close();
};

class RecentsInstance {
private:
  AppState *state;
  Glib::RefPtr<Gtk::Window> win;
  Glib::RefPtr<Gtk::Box> recent_files_box;

public:
  RecentsInstance(AppState *state);
  ~RecentsInstance();
  void show(const Glib::VariantBase &parameter);
  void close();
  bool lauch_by_action();
};

class PlayerInstance {
private:
  AppState *state;
  Glib::RefPtr<Gtk::Window> win;
  Glib::RefPtr<Gtk::Label> title;

public:
  PlayerInstance(AppState *state);
  ~PlayerInstance();
  void show();
  void load_song();
  bool lauch_by_action();
};

#define MODELS_H
#endif // !MODELS_H
