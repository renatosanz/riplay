#include "adwaita.h"
#include "audio/audio_manager.h"
#include "giomm/simpleaction.h"
#include "glib.h"
#include "glibmm/ustring.h"
#include "gtk/gtk.h"
#include "gtkmm/application.h"
#include "gtkmm/window.h"
#include "models/models.h"
#include "sigc++/functors/mem_fun.h"
#include "utils.h"
#include <iostream>
#include <memory>

AppState::AppState(char **argv, int argc)
    : Gtk::Application("org.riprtx.riplay",
                       Gio::Application::Flags::HANDLES_OPEN),
      argv(argv), argc(argc) {
  // add custom icon
  GdkDisplay *display = gdk_display_get_default();
  GtkIconTheme *icon_theme = gtk_icon_theme_get_for_display(display);
  gtk_icon_theme_add_resource_path(icon_theme, "/org/riplay/data/img");

  m_audioManager.setEOSCallback([this]() {
    m_info.state = PlaybackState::STOPPED;
    notifyListeners();
  });
}

void AppState::exit_app(const Glib::VariantBase &parameter) { this->quit(); }

AppState::~AppState() { printf("Clean up and closing ~AppState()...\n"); }

void AppState::on_open(const Gio::Application::type_vec_files &files,
                       const Glib::ustring &hint) {
  printf("Starting app on_open()...\n");
  load_views();
  load_actions();

  if (argc >= 2 && files.size() >= 1) {
    std::string path = files[0]->get_path();
    std::cout << "Opening single file: " << path << std::endl;
    open_player(path);
  } else {
    for (const auto &file : files) {
      std::cout << "Opening: " << file->get_path() << std::endl;
      // Handle multiple files
    }
  }
}

void AppState::on_activate() {
  printf("Starting app on_activate()...\n");
  if (argc == 1) {
    std::cout << "Activate: No files provided, showing home\n";
    load_views();
    load_actions();
    home->show();
  }
}

void AppState::load_views() {
  printf("Loading view load_views()...\n");
  home = std::make_unique<HomeInstance>(this);
  recents = std::make_unique<RecentsInstance>(this);
  player = std::make_unique<PlayerInstance>(this);
}

std::shared_ptr<SongInstance> AppState::get_song() { return current_song; };

void AppState::load_actions() {
  printf("Loading actions load_actions()...\n");
  // Create action for opening recent files
  auto recents_action_obj = Gio::SimpleAction::create("open-recents");
  auto open_new_file_action_obj = Gio::SimpleAction::create("open-new-file");
  auto toggle_play_action_obj = Gio::SimpleAction::create("toggle-play");
  auto exit_app_action_obj = Gio::SimpleAction::create("exit-app");
  auto open_keymaps_action_obj = Gio::SimpleAction::create("open-keymaps");
  auto open_credits_action_obj = Gio::SimpleAction::create("open-credits");

  recents_action_obj->signal_activate().connect(
      sigc::mem_fun(*recents, &RecentsInstance::show));
  open_new_file_action_obj->signal_activate().connect(
      sigc::mem_fun(*home, &HomeInstance::open_new_file));
  toggle_play_action_obj->signal_activate().connect(
      sigc::mem_fun(m_audioManager, &AudioManager::toggle_play));
  exit_app_action_obj->signal_activate().connect(
      sigc::mem_fun(*this, &AppState::exit_app));
  open_keymaps_action_obj->signal_activate().connect(
      sigc::mem_fun(*this, &AppState::show_keymaps_win));
  open_credits_action_obj->signal_activate().connect(
      sigc::mem_fun(*this, &AppState::show_credits_win));

  add_action(recents_action_obj);
  add_action(open_new_file_action_obj);
  add_action(toggle_play_action_obj);
  add_action(exit_app_action_obj);
  add_action(open_keymaps_action_obj);
  add_action(open_credits_action_obj);
  // // Create action for changing visual effects
  // GSimpleAction *visuals_action_obj =
  //     g_simple_action_new("change-visuals", NULL);
  // // Create action for opening new files
  // GSimpleAction *open_new_file_action_obj =
  //     g_simple_action_new("open-new-file", NULL);
  // // Create action for opening equalizer window
  // GSimpleAction *open_equalizer_action_obj =
  //     g_simple_action_new("open-equalizer", NULL);
  // GSimpleAction *enable_equalizer_action_obj =
  //     g_simple_action_new("enable-equalizer", NULL);
  // GSimpleAction *close_equalizer_action_obj =
  //     g_simple_action_new("close-equalizer", NULL);
  //
  // // Connect action signals to their handlers
  // g_signal_connect(recents_action_obj, "activate",
  //                  G_CALLBACK(recents->lauch_by_action), this->recents);
  // // g_signal_connect(visuals_action_obj, "activate",
  // //                  G_CALLBACK(open_visuals_menu), app_data);
  // // g_signal_connect(open_new_file_action_obj, "activate",
  // //                  G_CALLBACK(open_new_file_dialog), app_data);
  // // g_signal_connect(open_equalizer_action_obj, "activate",
  // //                  G_CALLBACK(open_equalizer), app_data);
  // // g_signal_connect(enable_equalizer_action_obj, "activate",
  // //                  G_CALLBACK(toggle_enable_equalizer), app_data);
  // // g_signal_connect(close_equalizer"activate",
  // //                  G_CALLBACK(close_equalizer), app_data);
  //
  // // Add actions to the application's action map
  // GActionMap *action_map = G_ACTION_MAP(app);
  // g_action_map_add_action(action_map, G_ACTION(recents_action_obj));
  // g_action_map_add_action(action_map, G_ACTION(visuals_action_obj));
  // g_action_map_add_action(action_map, G_ACTION(open_new_file_action_obj));
  // g_action_map_add_action(action_map, G_ACTION(open_equalizer_action_obj));
  // g_action_map_add_action(action_map, G_ACTION(enable_equalizer_action_obj));
  // g_action_map_add_action(action_map, G_ACTION(close_equalizer_action_obj));
}

void AppState::open_player(Glib::ustring filepath) {
  recents->close();
  player->close();
  home->close();
  m_audioManager = AudioManager();
  this->current_song = std::make_shared<SongInstance>(filepath);
  player->show();
}

void AppState::set_current_filename(gchar *f) { filename = f; }
gchar *AppState::get_current_filename() { return filename; }


void AppState::show_keymaps_win(const Glib::VariantBase &parameter) {
  auto builder = load_builder("/org/riplay/data/ui/keymaps.ui");
  keymaps_win = builder->get_object<Gtk::Window>("keymaps_win");
  add_window(*keymaps_win);
  keymaps_win->show();
}

void AppState::show_credits_win(const Glib::VariantBase &parameter) {
  const char *developers[] = {"Renato Sanchez", NULL};

  const char *designers[] = {"Renato Sanchez", NULL};

  adw_show_about_dialog(
      GTK_WIDGET(gtk_application_get_active_window(this->gobj())),
      "application-name", "Riplay", "application-icon", "icon.svg", "version",
      "0.1", "copyright", "© 2026 Renato Sanchez", "issue-url",
      "https://github.com/renatosanz/riplay/issues", "license-type",
      GTK_LICENSE_GPL_3_0, "developers", developers, "designers", designers,
      "translator-credits", "translator-credits", "website",
      "https://github.com/renatosanz/riplay", NULL);
}
