#ifndef MODELS_H
#define MODELS_H

#include "glib.h"
#include "glibmm/refptr.h"
#include "gtk/gtk.h"
#include "gtkmm/application.h"
#include "gtkmm/builder.h"
#include "gtkmm/drawingarea.h"
#include "gtkmm/filedialog.h"
#include "gtkmm/label.h"
#include "gtkmm/mediacontrols.h"
#include "gtkmm/mediastream.h"
#include "gtkmm/picture.h"
#include "gtkmm/widget.h"
#include "gtkmm/window.h"
#include "types.h"
#include <gtkmm/box.h>
#include <memory>
#include <string>

// defs
class AppState;
class HomeInstance;
class RecentsInstance;
class PlayerInstance;
class SongInstance;

// state
class AppState : public Gtk::Application {
private:
  GtkWidget *media_controls;
  GtkWidget *lyrics_label;
  GtkMediaStream *media_stream;
  float *audio_data;
  int data_size;
  char *filename;
  char **argv;
  int argc;
  bool files_were_opened = false;

  std::shared_ptr<SongInstance> current_song;

  std::unique_ptr<HomeInstance> home;
  std::unique_ptr<RecentsInstance> recents;
  std::unique_ptr<PlayerInstance> player;

  void load_actions();
  void load_views();

protected:
  void on_open(const Gio::Application::type_vec_files &files,
               const Glib::ustring &hint) override;
  void on_activate() override;

public:
  AppState(char **argv, int argc);
  ~AppState();
  std::shared_ptr<SongInstance> get_song();
  void open_player(Glib::ustring filepath);

  void set_current_filename(gchar *);
  gchar *get_current_filename();
};

class SongInstance {

private:
  std::string filepath;
  std::string raw_lyrics;
  std::shared_ptr<FileMetadata> metadata;
  std::vector<LyricBar> sync_lyrics;
  std::vector<LyricProp> lyric_props;

public:
  SongInstance(std::string filepath);
  std::string get_filepath();
  std::vector<LyricBar> get_sync_lyrics();
  std::vector<LyricProp> get_lyrics_props();
  std::string get_raw_lyrics();
  std::shared_ptr<FileMetadata> get_metadata();
  // ~SongInstance();
};

// instances
class HomeInstance {
private:
  AppState *state;
  Glib::RefPtr<Gtk::Window> win;
  Glib::RefPtr<Gtk::DrawingArea> drawing_area;
  Glib::RefPtr<Gtk::FileDialog> open_new_file_dialog;
  sigc::connection timeout_id;
  int position;

  void draw_stand_by_function(const std::shared_ptr<Cairo::Context> &cr,
                              int width, int height);
  bool on_timeout();

  void file_dialog_response(Glib::RefPtr<Gio::AsyncResult> &);

public:
  HomeInstance(AppState *state);
  ~HomeInstance();
  void show();
  void close();
  void open_new_file(const Glib::VariantBase &parameter);
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
  Glib::RefPtr<Gtk::Label> title_label;
  Glib::RefPtr<Gtk::Box> metadata_side;
  Glib::RefPtr<Gtk::Label> lyrics_label;
  Glib::RefPtr<Gtk::Picture> albumart_picture;
  Glib::RefPtr<Gtk::MediaStream> media_stream;
  Glib::RefPtr<Gtk::MediaControls> media_controls;

  std::string artis_label_format;
  std::string properties_format;
  std::string path_format;
  std::string date_format;
  std::string gender_format;
  std::string artist_format;

  gboolean lyrics_visible = false;
  gboolean metadata_side_visible = false;
  std::shared_ptr<FileMetadata> metadata;
  void setup_labels(Glib::RefPtr<Gtk::Builder> builder);
  void setup_button_actions(Glib::RefPtr<Gtk::Builder> builder);
  void setup_albumart(Glib::RefPtr<Gtk::Builder> builder);
  void setup_lyrics(Glib::RefPtr<Gtk::Builder> builder);
  void setup_metadata_side(Glib::RefPtr<Gtk::Builder> builder);

public:
  PlayerInstance(AppState *state);
  ~PlayerInstance();
  void show();
  void load_song();
  void close();
  bool lauch_by_action();
};

#define MODELS_H
#endif // !MODELS_H
