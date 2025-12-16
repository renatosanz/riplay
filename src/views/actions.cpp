#include "equalizer.h"
#include "gio/gio.h"
#include "glib-object.h"
#include "home.h"
#include "recents.h"
#include "types.h"
#include "visuals.h"

void load_actions(AppData *app_data) {
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
  g_signal_connect(recents_action_obj, "activate",
                   G_CALLBACK(open_recent_files), app_data);
  g_signal_connect(visuals_action_obj, "activate",
                   G_CALLBACK(open_visuals_menu), app_data);
  g_signal_connect(open_new_file_action_obj, "activate",
                   G_CALLBACK(open_new_file_dialog), app_data);
  g_signal_connect(open_equalizer_action_obj, "activate",
                   G_CALLBACK(open_equalizer), app_data);
  g_signal_connect(enable_equalizer_action_obj, "activate",
                   G_CALLBACK(toggle_enable_equalizer), app_data);
  g_signal_connect(close_equalizer_action_obj, "activate",
                   G_CALLBACK(close_equalizer), app_data);

  // Add actions to the application's action map
  GActionMap *action_map = G_ACTION_MAP(app_data->app);
  g_action_map_add_action(action_map, G_ACTION(recents_action_obj));
  g_action_map_add_action(action_map, G_ACTION(visuals_action_obj));
  g_action_map_add_action(action_map, G_ACTION(open_new_file_action_obj));
  g_action_map_add_action(action_map, G_ACTION(open_equalizer_action_obj));
  g_action_map_add_action(action_map, G_ACTION(enable_equalizer_action_obj));
  g_action_map_add_action(action_map, G_ACTION(close_equalizer_action_obj));
}
