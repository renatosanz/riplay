/**
 * @file main.c
 * @brief Main application file for Riplay audio player
 *
 * This file contains the main entry point and application lifecycle handlers
 * for the Riplay audio player. It manages application initialization, window
 * creation, and resource cleanup.
 */

#include "riplay.h"
#include "gui.h"
#include "metadata.h"

static int on_playing(GApplication *app, char *hint);

// Global application data structure
AppData *app_data;

// Global GTK application instance
GtkApplication *app_global;

// Project name constant
#define PROJECT_NAME "Riplay"

/**
 * @brief Application activate handler (no file playing)
 *
 * This function is called when the application is activated without a file.
 * It loads the home window where users can open a recent or new file.
 *
 * @param app The GApplication instance
 * @param hint Activation hint (unused)
 * @return int EXIT_SUCCESS on success, EXIT_FAILURE on error
 */
static int on_activate(GApplication *app, char *hint) {
  (void)hint;
  if (load_home_window(app, app_data)) {
    g_critical("Error opening window");
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

/**
 * @brief Clean up current session and start new one with different file
 *
 * This function handles the case when there's a song already playing and
 * the player wants to play a different song. It frees global resources
 * and creates a new session with the new song.
 *
 * @param filename Path to the new audio file to play
 * @return int EXIT_SUCCESS on success, EXIT_FAILURE on error
 */
int clean_new_on_playing(const char *filename) {
  if (!filename || !app_global) {
    g_critical("Invalid parameters");
    return EXIT_FAILURE;
  }

  if (app_data->pipeline) {
    gst_element_set_state(app_data->pipeline, GST_STATE_NULL);
    gst_object_unref(app_data->pipeline);
  }

  g_mutex_clear(&app_data->data_mutex);
  g_clear_pointer(&app_data->audio_data, g_free);

  // Clean up existing window and resources
  gtk_window_destroy(GTK_WINDOW(app_data->win));
  g_object_unref(app_data->win);

  free_global_resources(app_data);

  // Initialize new application data
  app_data = g_new0(AppData, 1);
  app_data->filename = g_strdup(filename);

  if (!app_data->filename) {
    g_critical("Memory allocation failed for filename");
    free_global_resources(app_data);
    return EXIT_FAILURE;
  }

  if (on_playing(NULL, (char *)filename)) {
    g_critical("Error running new session.");
    free_global_resources(app_data);
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

/**
 * @brief Application activate handler (with file playing)
 *
 * This function is called when the application is activated with a file
 * to play immediately.
 *
 * @param app The GApplication instance
 * @param hint Activation hint (unused)
 * @return int EXIT_SUCCESS on success, EXIT_FAILURE on error
 */
static int on_playing(GApplication *app, char *hint) {
  (void)hint;

  // Get metadata for the file
  app_data->metadata = get_metadata(app_data->filename);
  if (!app_data->metadata) {
    g_critical("Error getting metadata from %s", app_data->filename);
    return EXIT_FAILURE;
  }

  // Load player window
  if (load_player_window(G_APPLICATION(app_global), app_data)) {
    g_critical("Error opening window");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

/**
 * @brief Application shutdown handler
 *
 * Cleans up global resources when the application is shutting down.
 */
static void on_shutdown() {
  free_global_resources(app_data);
  g_print("Application shutdown\n");
}

/**
 * @brief File open handler
 *
 * Handles opening files when the application is launched with file arguments
 * or when files are opened through the file manager.
 *
 * @param app The GApplication instance
 * @param files Array of GFile objects to open
 * @param n_files Number of files in the array
 * @param hint Open hint (unused)
 * @return int EXIT_SUCCESS on success
 */
static int on_open_file(GApplication *app, GFile **files, int n_files,
                        char *hint) {
  if (n_files == 1) {
    // Single file - play immediately
    on_playing(app, hint);
  } else {
    // Multiple files - currently just prints paths
    for (int i = 0; i < n_files; i++) {
      char *path = g_file_get_path(files[i]);
      g_print("Opening file: %s\n", path);
      // TODO: Implement multi-file handling
      g_free(path);
    }
  }
  return EXIT_SUCCESS;
}

/**
 * @brief Main application entry point
 *
 * Initializes the application and starts the main event loop.
 *
 * @param argc Argument count
 * @param argv Argument vector
 * @return int Application exit status
 */
int main(int argc, char **argv) {
  gtk_init();

  // initialize application data
  app_data = g_new0(AppData, 1);
  app_data->metadata = NULL;

  // handle command line arguments
  if (argc == 2) {
    app_data->filename = g_strdup(argv[1]);
    g_print("Opening file: %s\n", app_data->filename);
  }

  // create and configure GTK application
  app_global =
      gtk_application_new("org.riprtx.riplay", G_APPLICATION_HANDLES_OPEN);
  gtk_window_set_default_icon_name(PROJECT_NAME);

  // connect signal handlers
  g_signal_connect(app_global, "activate", G_CALLBACK(on_activate), NULL);
  g_signal_connect(app_global, "open", G_CALLBACK(on_open_file), NULL);
  g_signal_connect(app_global, "shutdown", G_CALLBACK(on_shutdown), NULL);

  // run app
  int status = g_application_run(G_APPLICATION(app_global), argc, argv);

  // cleanup
  g_object_unref(app_global);

  return status;
}
