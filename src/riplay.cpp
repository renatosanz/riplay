#include "riplay.h"
#include "gui.h"
#include "metadata.h"

static int on_playing(GApplication *app, char *hint);
AppData *app_data;
GtkApplication *app_global;
#define PROJECT_NAME "Riplay"

static int on_activate(GApplication *app, char *hint) {
  (void)hint;
  if (load_home_window(app, app_data)) {
    g_critical("Error opening window");
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

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

  gtk_window_destroy(GTK_WINDOW(app_data->win));
  g_object_unref(app_data->win);

  free_global_resources(app_data);

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

static int on_playing(GApplication *app, char *hint) {
  (void)hint;

  app_data->metadata = get_metadata(app_data->filename);
  if (!app_data->metadata) {
    g_critical("Error getting metadata from %s", app_data->filename);
    return EXIT_FAILURE;
  }

  if (load_player_window(G_APPLICATION(app_global), app_data)) {
    g_critical("Error opening window");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

static void on_shutdown() {
  free_global_resources(app_data);
  g_print("Application shutdown\n");
}

static int on_open_file(GApplication *app, GFile **files, int n_files,
                        char *hint) {
  if (n_files == 1) {
    on_playing(app, hint);
  } else {
    for (int i = 0; i < n_files; i++) {
      char *path = g_file_get_path(files[i]);
      g_print("Opening file: %s\n", path);
      // TODO: Implement multi-file handling
      g_free(path);
    }
  }
  return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
  gtk_init();

  app_data = g_new0(AppData, 1);
  app_data->metadata = NULL;
  
  if (argc == 2) {
    app_data->filename = g_strdup(argv[1]);
    g_print("Opening file: %s\n", app_data->filename);
  }

  app_global =
      gtk_application_new("org.riprtx.riplay", G_APPLICATION_HANDLES_OPEN);
  gtk_window_set_default_icon_name(PROJECT_NAME);

  g_signal_connect(app_global, "activate", G_CALLBACK(on_activate), NULL);
  g_signal_connect(app_global, "open", G_CALLBACK(on_open_file), NULL);
  g_signal_connect(app_global, "shutdown", G_CALLBACK(on_shutdown), NULL);

  int status = g_application_run(G_APPLICATION(app_global), argc, argv);

  g_object_unref(app_global);

  return status;
}
