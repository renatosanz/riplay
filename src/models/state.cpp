#include "models/models.h"
AppState::AppState(GtkApplication *app, char **argv, int argc) {
  this->app = app;
  this->argv = argv;
  this->argc = argc;
  if (argc == 2) {
    set_current_filename(g_strdup(argv[1]));
    g_print("Opening file: %s\n", get_current_filename());
  }

  g_signal_connect(app, "activate", G_CALLBACK(on_activate_callback), this);
  // g_signal_connect(app, "open", G_CALLBACK(on_open_file), NULL);
  // g_signal_connect(app, "shutdown", G_CALLBACK(on_shutdown), NULL);
}

AppState::~AppState() {
  printf("Clean up and closing ~AppState()...\n");

  g_object_unref(app);
}

void AppState::on_activate_callback(GtkApplication *app, gpointer user_data) {
  AppState *self = static_cast<AppState *>(user_data);
  self->on_activate();
}

void AppState::on_activate() {
  printf("Starting app on_activate()...\n");
  load_actions();
  load_views();
  // open home on default start
  home->show();
}

void AppState::load_views() {
  printf("Loading view load_views()...\n");
  home = std::make_unique<HomeInstance>(this);
}

GtkApplication *AppState::get_app() { return app; };

void AppState::load_actions() {
  printf("Loading actions load_actions()...\n");
  // Create action for opening recent files
  GSimpleAction *recents_action_obj = g_simple_action_new("open-recents", NULL);
  // Create action for changing visual effects
  GSimpleAction *visuals_action_obj =
      g_simple_action_new("change-visuals", NULL);
  // Create action for opening new files
  GSimpleAction *open_new_file_action_obj =
      g_simple_action_new("open-new-file", NULL);
  // Create action for opening equalizer window
  GSimpleAction *open_equalizer_action_obj =
      g_simple_action_new("open-equalizer", NULL);
  GSimpleAction *enable_equalizer_action_obj =
      g_simple_action_new("enable-equalizer", NULL);
  GSimpleAction *close_equalizer_action_obj =
      g_simple_action_new("close-equalizer", NULL);

  // Connect action signals to their handlers
  // g_signal_connect(recents_action_obj, "activate",
  //                  G_CALLBACK(open_recent_files), app_data);
  // g_signal_connect(visuals_action_obj, "activate",
  //                  G_CALLBACK(open_visuals_menu), app_data);
  // g_signal_connect(open_new_file_action_obj, "activate",
  //                  G_CALLBACK(open_new_file_dialog), app_data);
  // g_signal_connect(open_equalizer_action_obj, "activate",
  //                  G_CALLBACK(open_equalizer), app_data);
  // g_signal_connect(enable_equalizer_action_obj, "activate",
  //                  G_CALLBACK(toggle_enable_equalizer), app_data);
  // g_signal_connect(close_equalizer_action_obj, "activate",
  //                  G_CALLBACK(close_equalizer), app_data);

  // Add actions to the application's action map
  GActionMap *action_map = G_ACTION_MAP(app);
  g_action_map_add_action(action_map, G_ACTION(recents_action_obj));
  g_action_map_add_action(action_map, G_ACTION(visuals_action_obj));
  g_action_map_add_action(action_map, G_ACTION(open_new_file_action_obj));
  g_action_map_add_action(action_map, G_ACTION(open_equalizer_action_obj));
  g_action_map_add_action(action_map, G_ACTION(enable_equalizer_action_obj));
  g_action_map_add_action(action_map, G_ACTION(close_equalizer_action_obj));
}

int AppState::run() {
  int status = g_application_run(G_APPLICATION(app), argc, argv);
  return status;
}
void AppState::set_current_filename(gchar *f) { filename = f; }
gchar *AppState::get_current_filename() { return filename; }
