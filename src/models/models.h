#ifndef MODELS_H
#define MODELS_H

#include "audio/audio_manager.h"
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
class LyricsManager;

struct PlaybackInfo {
  PlaybackState state{PlaybackState::STOPPED};
  int64_t currentPosition{0};
  int64_t duration{0};
  std::string currentTrackPath;
};

// state
class AppState : public Gtk::Application {
public:
  AppState(char **argv, int argc);
  ~AppState();
  void exit_app(const Glib::VariantBase &parameter);
  std::shared_ptr<SongInstance> get_song();
  void open_player(Glib::ustring filepath);

  void set_current_filename(gchar *);
  gchar *get_current_filename();

  // Acceso al reproductor de audio
  AudioManager &audio() { return m_audioManager; }

  // Métodos delegados para acciones de alto nivel
  void openAndPlay(const std::string &filePath) {
    if (m_audioManager.loadFile(filePath)) {
      m_info.currentTrackPath = filePath;
      m_audioManager.play();
      notifyListeners();
    }
  }

  // Suscripción de vistas (UI Components)
  using StateListener = std::function<void(const PlaybackInfo &)>;
  void subscribe(StateListener listener) { m_listeners.push_back(listener); }

  // Método para sincronizar la UI (ej. llamado periódicamente o desde un Timer)
  void tick() {
    m_info.state = m_audioManager.getState();
    m_info.currentPosition = m_audioManager.getPositionSeconds();
    m_info.duration = m_audioManager.getDurationSeconds();
    notifyListeners();
  }

private:
  GtkWidget *lyrics_label;
  GtkMediaStream *media_stream;
  int data_size;
  char *filename;
  char **argv;
  int argc;
  bool files_were_opened = false;

  const PlaybackInfo &getInfo() const { return m_info; }
  PlaybackInfo m_info;
  std::vector<StateListener> m_listeners;

  void notifyListeners() {
    for (const auto &listener : m_listeners) {
      listener(m_info);
    }
  }

  std::shared_ptr<SongInstance> current_song;

  std::unique_ptr<HomeInstance> home;
  std::unique_ptr<RecentsInstance> recents;
  std::unique_ptr<PlayerInstance> player;

  void load_actions();
  void load_views();

  AudioManager m_audioManager;

protected:
  void on_open(const Gio::Application::type_vec_files &files,
               const Glib::ustring &hint) override;
  void on_activate() override;
};

class SongInstance {

private:
  std::string filepath;
  std::shared_ptr<FileMetadata> metadata;

public:
  SongInstance(std::string filepath);
  std::string get_filepath();
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

  std::shared_ptr<LyricsManager> lyrics_manager;

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
