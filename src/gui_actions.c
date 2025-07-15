/**
 * @file gui_actions.c
 * @brief GUI action loading and builder utilities
 *
 * This module handles the initialization of application actions and
 * provides utility functions for loading UI builders from resources.
 */

#include "glib-object.h"
#include "gui.h"

/**
 * @brief Loads and connects all application actions
 *
 * Initializes the main application actions and connects them to their
 * respective callback functions. The actions are:
 * - "open-recents": Opens recent files dialog
 * - "change-visuals": Opens visual effects menu
 * - "open-new-file": Opens file selection dialog
 *
 * @param app The GApplication instance to which actions will be added
 */
void load_actions(GApplication *app) {
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

  // Connect action signals to their handlers
  g_signal_connect(recents_action_obj, "activate",
                   G_CALLBACK(open_recent_files), app);
  g_signal_connect(visuals_action_obj, "activate",
                   G_CALLBACK(open_visuals_menu), app);
  g_signal_connect(open_new_file_action_obj, "activate",
                   G_CALLBACK(open_new_file_dialog), app);
  g_signal_connect(open_equalizer_action_obj, "activate",
                   G_CALLBACK(open_equalizer), app);
  g_signal_connect(enable_equalizer_action_obj, "activate",
                   G_CALLBACK(toggle_enable_equalizer), app);

  // Add actions to the application's action map
  GActionMap *action_map = G_ACTION_MAP(app);
  g_action_map_add_action(action_map, G_ACTION(recents_action_obj));
  g_action_map_add_action(action_map, G_ACTION(visuals_action_obj));
  g_action_map_add_action(action_map, G_ACTION(open_new_file_action_obj));
  g_action_map_add_action(action_map, G_ACTION(open_equalizer_action_obj));
  g_action_map_add_action(action_map, G_ACTION(enable_equalizer_action_obj));
}

/**
 * @brief Loads a GtkBuilder from a resource path
 *
 * Creates a new GtkBuilder instance and loads the UI definition from
 * the specified resource path. The resource should be compiled into
 * the application's GResource bundle.
 *
 * @param path The resource path to the UI definition file
 * @return GtkBuilder* A new builder instance with the loaded UI
 *
 * @note The caller is responsible for unreferencing the returned builder
 */
GtkBuilder *load_builder(const char *path) {
  GtkBuilder *builder = gtk_builder_new();
  if (!gtk_builder_add_from_resource(builder, path, NULL)) {
    g_warning("Failed to load UI resource: %s", path);
    g_object_unref(builder);
    return NULL;
  }
  return builder;
}
