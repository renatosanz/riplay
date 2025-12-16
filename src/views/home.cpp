/**
 * @brief Loads and displays the home window
 *
 * Creates the application's home window where users can open recent files
 * or select new ones. Initializes a standby animation.
 *
 * @param app The GApplication instance
 * @param spectrum Pointer to store the spectrum drawing area
 * @param win Pointer to store the created window
 * @return int 0 on success, non-zero on error
 */
#include "file_history.h"
#include "glib-object.h"
#include "gtk/gtk.h"
#include "player.h"
#include "standby.h"
#include "types.h"
#include "utils.h"
#include <cstdio>

int show_home_view(AppData *app_data) {
  g_return_val_if_fail(app_data->app != NULL && app_data != NULL, -1);

  // load ui from resources
  GtkBuilder *builder = load_builder("/org/riplay/data/ui/home.ui");
  if (!builder) {
    g_critical("Failed to load home window UI");
    return -1;
  }

  // get window from builder
  app_data->home->win =
      GTK_WINDOW(gtk_builder_get_object(builder, "default_window"));
  GTK_WINDOW(gtk_builder_get_object(builder, "default_window"));
  if (!app_data->home->win) {
    g_critical("Failed to get home window from builder");
    g_object_unref(builder);
    return -1;
  }

  // set up standby animation
  app_data->home->drawing_area =
      GTK_DRAWING_AREA(gtk_builder_get_object(builder, "spectrum_default"));
  gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(app_data->home->drawing_area),
                                 draw_stand_by_function, NULL, NULL);

  // Start animation timer (32ms interval ≈ 30fps)
  app_data->home->timeout_id =
      g_timeout_add(32, on_timeout, app_data->home->drawing_area);

  // clean up builder
  g_object_unref(builder);

  // display window
  gtk_application_add_window(GTK_APPLICATION(app_data->app),
                             app_data->home->win);
  gtk_window_present(app_data->home->win);

  return 0;
}

void file_dialog_response(GObject *source_object, GAsyncResult *result,
                          AppData *app_data) {
  GtkFileDialog *dialog = GTK_FILE_DIALOG(source_object);
  GError *error = NULL;

  // Get selected file
  GFile *file = gtk_file_dialog_open_finish(dialog, result, &error);

  if (error) {
    g_printerr("File selection error: %s\n", error->message);
    g_error_free(error);
    return;
  }

  if (!file) {
    g_print("No file selected\n");
    return;
  }

  // Get file path
  char *filepath = g_file_get_path(file);
  if (!filepath) {
    g_object_unref(file);
    g_critical("Failed to get file path");
    return;
  }

  // add filepath to file history
  add_to_recent_files(filepath);

  // Start new playback session
  clean_new_on_playing(filepath);

  // Clean up
  g_free(filepath);
  g_object_unref(file);
}

/**
 * @brief Opens file selection dialog
 *
 * Creates and displays a file dialog for selecting audio files.
 *
 * @param action The action that triggered this (unused)
 * @param parameter Action parameter (unused)
 * @param app The application instance
 */
void open_new_file_dialog(GSimpleAction *action, GVariant *parameter,
                          AppData *app_data) {
  (void)action;
  (void)parameter;

  GtkWindow *window = GTK_WINDOW(app_data->home->win);
  GtkFileDialog *dialog = gtk_file_dialog_new();

  // Configure dialog
  gtk_file_dialog_set_title(dialog, "Select a song");

  // Set audio file filter
  GtkFileFilter *filter = gtk_file_filter_new();
  gtk_file_filter_add_mime_type(filter, "audio/*");
  gtk_file_dialog_set_default_filter(dialog, filter);

  // Show dialog
  gtk_file_dialog_open(dialog, window, NULL,
                       GAsyncReadyCallback(file_dialog_response), app_data);
}
