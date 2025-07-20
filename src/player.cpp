#include "gio/gio.h"
#include "gtk/gtk.h"
#include "gtk/gtkshortcut.h"
#include "gui.h"
#include <metadata.h>

static void draw_callback(GtkDrawingArea *area, cairo_t *cr, int width,
                          int height, gpointer user_data) {
  AppData *app = (AppData *)user_data;
  GdkRGBA color;

  cairo_set_antialias(cr, CAIRO_ANTIALIAS_BEST);
  // Set drawing properties
  cairo_set_source_rgb(cr, 1, 1, 1); // White color
  cairo_set_line_width(cr, 5);       // Line width
  cairo_paint(cr);

  if (app->audio_data && app->data_size > 0) {
    // Onda de audio en verde
    cairo_set_source_rgb(cr, 0, 1, 0);
    cairo_set_line_width(cr, 1.5);
    cairo_move_to(cr, 0, (float)height / 2);

    for (int i = 0; i < width; i++) {
      int index = (i * app->data_size) / width;
      float sample = app->audio_data[index];
      float y = ((float)height / 2) + (sample * ((float)height / 2));
      cairo_line_to(cr, i, y);
    }
    cairo_stroke(cr);
  }
  gtk_widget_get_color(GTK_WIDGET(area), &color);
  gdk_cairo_set_source_rgba(cr, &color);
  cairo_fill(cr);
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
  } else {
    if (app->audio_data)
      g_free(app->audio_data);
    app->audio_data = NULL;
    gtk_widget_queue_draw(app->drawing_area);
  }
}

static gboolean on_timeout_playing(gpointer user_data) {
  AppData *app = (AppData *)user_data;
  update_visualizer(app);
  return G_SOURCE_CONTINUE;
}

int load_player_window(GApplication *app, AppData *app_data) {
  load_actions(app);

  unsigned long size;
  unsigned char *img_data = extract_album_art(app_data->filename, &size);

  GtkBuilder *b = load_builder("/org/riplay/data/ui/player.ui");
  app_data->win = GTK_WINDOW(gtk_builder_get_object(b, "player_window"));

  if (app_data->metadata && app_data->filename) {
    app_data->media_stream =
        gtk_media_file_new_for_filename(app_data->filename);
    gtk_media_stream_set_playing(app_data->media_stream, true);

    const char *artis_label_format =
        "%s - %s"; // format for artist - album label
    const char *properties_format =
        " %d kbps - %d sec (%d:%02d) - %d Hz"; // format year label

    // get the labels from the builder
    GtkWidget *title_label =
        GTK_WIDGET(gtk_builder_get_object(b, "title_label"));
    GtkWidget *properties_label =
        GTK_WIDGET(gtk_builder_get_object(b, "year_label"));
    GtkWidget *artis_album_label =
        GTK_WIDGET(gtk_builder_get_object(b, "artis_album_label"));
    app_data->media_controls =
        GTK_WIDGET(gtk_builder_get_object(b, "audio_controls"));

    // set strings to labels
    gtk_label_set_ellipsize(GTK_LABEL(title_label), PANGO_ELLIPSIZE_END);
    gtk_label_set_ellipsize(GTK_LABEL(properties_label), PANGO_ELLIPSIZE_END);
    gtk_label_set_ellipsize(GTK_LABEL(artis_album_label), PANGO_ELLIPSIZE_END);

    gtk_label_set_label(GTK_LABEL(title_label), app_data->metadata->title);
    gtk_label_set_label(
        GTK_LABEL(properties_label),
        g_strdup_printf(
            properties_format, app_data->metadata->properties->bitrate,
            app_data->metadata->properties->length,
            (int)app_data->metadata->properties->length / MIN_IN_SECS,
            (int)app_data->metadata->properties->length % MIN_IN_SECS,
            app_data->metadata->properties->samplerate));
    gtk_label_set_label(GTK_LABEL(artis_album_label),
                        g_strdup_printf(artis_label_format,
                                        app_data->metadata->artist,
                                        app_data->metadata->album));

    gtk_media_controls_set_media_stream(
        GTK_MEDIA_CONTROLS(app_data->media_controls), app_data->media_stream);

    GtkBox *albumart_content =
        GTK_BOX(gtk_builder_get_object(b, "album_lyrics_cont"));

    if (img_data) {
      GdkPixbufLoader *loader = gdk_pixbuf_loader_new();
      gdk_pixbuf_loader_write(loader, img_data, size, NULL);
      gdk_pixbuf_loader_close(loader, NULL);
      GdkPixbuf *pixbuf = gdk_pixbuf_loader_get_pixbuf(loader);

      // Create a GTK image widget
      GtkWidget *image = gtk_image_new_from_pixbuf(pixbuf);
      gtk_widget_set_vexpand(image, TRUE);
      gtk_widget_set_hexpand(image, TRUE);
      gtk_box_append(albumart_content, image);
    }
    GtkWidget *lyrics = gtk_label_new("Lyrics");
    gtk_widget_set_vexpand(lyrics, TRUE);
    gtk_widget_set_hexpand(lyrics, TRUE);
    gtk_box_append(albumart_content, lyrics);
    // init visualizer
    g_object_unref(b);

    gtk_application_add_window(GTK_APPLICATION(app), GTK_WINDOW(app_data->win));
    gtk_window_present(GTK_WINDOW(app_data->win));
    return 0;
  } else {
    g_printerr("error loading player!");
    return EXIT_FAILURE;
  }
}
