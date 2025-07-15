#include "gui.h"

int load_player_window(GApplication *app, const char *filename,
                       GtkWidget **spectrum, GtkWindow **win,
                       GtkWidget **player, GtkMediaStream **stream,
                       FileMetaData *metadata)
{
  load_actions(app);

  GtkBuilder *b = load_builder("/org/riplay/data/ui/player.ui");
  *win = GTK_WINDOW(gtk_builder_get_object(b, "player_window"));

  if (metadata && filename)
  {
    *stream = gtk_media_file_new_for_filename(filename);
    // labels
    const char *artis_label_format =
        "%s - %s"; // format for artist - album label
    const char *properties_format =
        " %d kbps - %d sec (%d:%02d) - %d Hz"; // format year label
    GtkWidget *title_label;                    // title song label
    GtkWidget *artis_album_label;              // label for artist - album
    GtkWidget *properties_label;               // label for song year

    // get the labels from the builder
    title_label = GTK_WIDGET(gtk_builder_get_object(b, "title_label"));
    properties_label = GTK_WIDGET(gtk_builder_get_object(b, "year_label"));
    artis_album_label =
        GTK_WIDGET(gtk_builder_get_object(b, "artis_album_label"));
    *player = GTK_WIDGET(gtk_builder_get_object(b, "audio_controls"));

    // set strings to labels
    gtk_label_set_ellipsize(GTK_LABEL(title_label), PANGO_ELLIPSIZE_END);
    gtk_label_set_ellipsize(GTK_LABEL(properties_label), PANGO_ELLIPSIZE_END);
    gtk_label_set_ellipsize(GTK_LABEL(artis_album_label), PANGO_ELLIPSIZE_END);

    gtk_label_set_label(GTK_LABEL(title_label), metadata->title);
    gtk_label_set_label(
        GTK_LABEL(properties_label),
        g_strdup_printf(properties_format, metadata->properties->bitrate,
                        metadata->properties->length,
                        (int)metadata->properties->length / MIN_IN_SECS,
                        (int)metadata->properties->length % MIN_IN_SECS,
                        metadata->properties->samplerate));
    gtk_label_set_label(
        GTK_LABEL(artis_album_label),
        g_strdup_printf(artis_label_format, metadata->artist, metadata->album));

    gtk_media_controls_set_media_stream(GTK_MEDIA_CONTROLS(*player), *stream);
    // init visualizer
    *spectrum = GTK_WIDGET(gtk_builder_get_object(b, "spectrum_viewer"));
    // gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(spectrum),
    // draw_stand_by_function,
    //                                NULL, NULL);

    // g_timeout_add(32, on_timeout, spectrum); // draw spectrum_viewer
    g_object_unref(b);

    gtk_application_add_window(GTK_APPLICATION(app), GTK_WINDOW(*win));
    gtk_window_present(GTK_WINDOW(*win));
    return 0;
  }
  else
  {
    g_printerr("error loading player!");
    return EXIT_FAILURE;
  }
}
