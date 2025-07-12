#include "gio/gio.h"
#include "glib.h"
#include "types.h"
#include <SDL2/SDL.h>
#include <gtk/gtk.h>
#include <gtk_utils.h>
#include <metadata.h>
#include <mpg123.h>
#include <riplay.h>
#include <stdio.h>
#include <stdlib.h>

AppData *app_data;

GtkApplication *app_global;

#define PROJECT_NAME "riplay"

void free_global_resources();

// default window (no playing song) for open a recent or new file
static int on_activate(GApplication *app, char *hint) {
  (void)hint;

  if (load_default_window(app, app_data->filename, &app_data->drawing_area,
                          &app_data->win, &app_data->media_controls,
                          app_data->metadata)) {
    printf("Error oppening window\n");
    return EXIT_FAILURE;
  }
  return 0;
}

int clean_new_on_playing(const char *filename) {
  if (!filename || !app_global) {
    g_critical("Parámetros inválidos");
    return EXIT_FAILURE;
  }

  gtk_window_destroy(GTK_WINDOW(app_data->win));
  g_object_unref(app_data->win);

  free_global_resources();

  app_data = g_new0(AppData, 1);
  app_data->filename = g_strdup(filename);

  if (!app_data->filename) {
    g_critical("Fallo al asignar memoria para filename");
    free_global_resources();
    return EXIT_FAILURE;
  }

  app_data->metadata = get_metadata(app_data->filename);
  if (!app_data->metadata) {
    g_critical("Error obteniendo metadatos para: %s", filename);
    free_global_resources();
    return EXIT_FAILURE;
  }

  if (load_player_window(G_APPLICATION(app_global), app_data->filename,
                         &app_data->drawing_area, &app_data->win,
                         &app_data->media_controls, &app_data->media_stream,
                         app_data->metadata)) {
    g_critical("Error cargando ventana de reproducción");
    free_global_resources();
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

// open playing a song
static int on_playing(GApplication *app, char *hint) {
  (void)hint;

  app_data->metadata = get_metadata(app_data->filename);
  if (!app_data->metadata) {
    printf("Error getting metadata\n");
    return EXIT_FAILURE;
  }

  if (load_player_window(app, app_data->filename, &app_data->drawing_area,
                         &app_data->win, &app_data->media_controls,
                         &app_data->media_stream, app_data->metadata)) {
    printf("Error oppening window\n");
    return EXIT_FAILURE;
  }

  return 0;
}

void free_global_resources() {

  if (app_data->media_stream && G_IS_OBJECT(app_data->media_stream)) {
    g_object_unref(app_data->media_stream);
    app_data->media_stream = NULL;
  }

  if (!app_data)
    return;

  if (app_data->metadata) {
    if (app_data->metadata->propieties) {
      g_free(app_data->metadata->propieties);
      app_data->metadata->propieties = NULL;
    }

    g_free(app_data->metadata);
    app_data->metadata = NULL;
  }

  if (app_data->filename) {
    g_free(app_data->filename);
    app_data->filename = NULL;
  }

  g_free(app_data);
  app_data = NULL;
}

// app shutdown (free global resources)
static void on_shutdown() {
  free_global_resources();
  g_print("shutdown app\n");
}

static int on_open_file(GApplication *app, GFile **files, int n_files,
                        char *hint) {
  if (n_files == 1) {
    on_playing(app, hint);
  } else {
    for (int i = 0; i < n_files; i++) {
      char *path = g_file_get_path(files[i]);
      printf("Abriendo archivo: %s\n", path);
      // Aquí tu lógica para reproducir el archivo
      g_free(path);
    }
  }
  return 0;
}

int main(int argc, char **argv) {
  gtk_init();

  app_data = g_new0(AppData, 1);
  app_data->metadata = NULL;

  if (argc == 2) {
    app_data->filename = g_strdup(argv[1]);
    g_print("%s", app_data->filename);
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
