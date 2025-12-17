#include "actions.h"
#include "glib.h"
#include "gtk/gtk.h"
#include "gtkmm/application.h"
#include "home.h"
#include "models/models.h"
#include "player.h"
#include "types.h"
#include "utils.h"
#include <cstdio>
#include <metadata/metadata.h>

// static int on_playing(AppData *app_data) {
//
//   printf("looking for metadata...\n");
//   app_data->metadata = get_metadata(app_data->filename);
//   if (!app_data->metadata) {
//     g_critical("Error getting metadata from %s", app_data->filename);
//     return EXIT_FAILURE;
//   }
//   printf("metadata extracted successfully\n");
//
//   if (load_player_window(app_data)) {
//     g_critical("Error opening window");
//     return EXIT_FAILURE;
//   }
//   printf("player loaded successfully\n");
//
//   return EXIT_SUCCESS;
// }
//
// static int on_activate(GApplication *app, char *hint) {
//   (void)hint;
//
//   // load actions for ui buttons
//   load_actions(app_data);
//
//   if (show_home_view(app_data)) {
//     g_critical("Error opening window");
//     return EXIT_FAILURE;
//   }
//   return EXIT_SUCCESS;
// }
//
// int clean_new_on_playing(const char *filename) {
//
//   // Stop any current playback
//   if (app_data->home->timeout_id > 0) {
//     g_source_remove(app_data->home->timeout_id);
//     app_data->home->timeout_id = 0;
//   }
//
//   // Start playback of selected file
//
//   if (app_data->recents->win) {
//     gtk_window_close(app_data->recents->win);
//   }
//
//   if (!filename || !app_data->app) {
//     g_critical("Invalid parameters");
//     return EXIT_FAILURE;
//   }
//
//   g_mutex_clear(&app_data->data_mutex);
//   g_clear_pointer(&app_data->audio_data, g_free);
//
//   gtk_window_close(GTK_WINDOW(app_data->home->win));
//
//   // stop_lyrics_display();
//   // free_global_resources(app_data);
//
//   // app_data = g_new0(AppData, 1);
//   app_data->filename = g_strdup(filename);
//
//   if (!app_data->filename) {
//     g_critical("Memory allocation failed for filename");
//     free_global_resources(app_data);
//     return EXIT_FAILURE;
//   }
//
//   printf("filename placed: %s\n", app_data->filename);
//
//   if (on_playing(app_data)) {
//     g_critical("Error running new session.");
//     free_global_resources(app_data);
//     return EXIT_FAILURE;
//   }
//
//   return EXIT_SUCCESS;
// }
//
// static void on_shutdown() {
//   free_global_resources(app_data);
//   // stop_lyrics_display();
//   g_print("Application shutdown\n");
// }
//
// static int on_open_file(GApplication *app, GFile **files, int n_files,
//                         char *hint) {
//   if (n_files == 1) {
//     on_playing(app_data);
//   } else {
//     for (int i = 0; i < n_files; i++) {
//       char *path = g_file_get_path(files[i]);
//       g_print("Opening file: %s\n", path);
//       // TODO: Implement multi-file handling
//       g_free(path);
//     }
//   }
//   return EXIT_SUCCESS;
// }

int main(int argc, char **argv) {
  AppState *state =
      new AppState(Gtk::Application::create("org.riprtx.riplay"), argv, argc);

  return state->run();
}
