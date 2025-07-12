#include "redering.h"
#include "types.h"
#define WIDTH 800
#define HEIGHT 300

static gboolean draw_callback(GtkDrawingArea *area, cairo_t *cr, int width,
                              int height, gpointer user_data) {
  AppData *app = (AppData *)user_data;

  // Fondo negro
  cairo_set_source_rgb(cr, 0, 0, 0);
  cairo_paint(cr);

  if (app->audio_data && app->data_size > 0) {
    // Onda de audio en verde
    cairo_set_source_rgb(cr, 0, 1, 0);
    cairo_set_line_width(cr, 1.5);
    cairo_move_to(cr, 0, height / 2);

    for (int i = 0; i < width; i++) {
      int index = (i * app->data_size) / width;
      float sample = app->audio_data[index];
      float y = (height / 2) + (sample * (height / 2));
      cairo_line_to(cr, i, y);
    }
    cairo_stroke(cr);
  }

  return TRUE;
}

static void update_visualizer(AppData *app) {
  // Aquí iría la lógica para obtener los datos de audio actuales
  // Esto es un ejemplo simplificado
  if (app->media_stream && gtk_media_stream_get_playing(app->media_stream)) {
    // Simular datos de audio (en una implementación real usarías GStreamer)
    if (app->audio_data)
      g_free(app->audio_data);
    app->data_size = 1000;
    app->audio_data = g_new(float, app->data_size);

    for (int i = 0; i < app->data_size; i++) {
      app->audio_data[i] = sin(i * 0.1) * 0.5;
    }

    gtk_widget_queue_draw(app->drawing_area);
  }
}

static gboolean on_timeout(gpointer user_data) {
  AppData *app = (AppData *)user_data;
  update_visualizer(app);
  return G_SOURCE_CONTINUE;
}

static void setup_media_player(AppData *app, const char *filepath) {
  // Crear el media stream
  app->media_stream = gtk_media_file_new_for_filename(filepath);

  // Configurar controles de medios
  app->media_controls = gtk_media_controls_new(app->media_stream);

  // Configurar actualización periódica del visualizador
  app->timeout_id = g_timeout_add(50, on_timeout, app);

  // Iniciar reproducción automática
  gtk_media_stream_play(app->media_stream);
}
