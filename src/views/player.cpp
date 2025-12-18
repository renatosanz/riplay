#include "player.h"
#include "actions.h"
#include "gdkmm/pixbuf.h"
#include "gdkmm/texture.h"
#include "gio/gio.h"
#include "glib-object.h"
#include "glib.h"
#include "gtkmm.h"
#include "gtkmm/label.h"
#include "gtkmm/mediacontrols.h"
#include "gtkmm/mediafile.h"
#include "gtkmm/object.h"
#include "gtkmm/picture.h"
#include "models/models.h"
#include "mpegfile.h"
#include "pango/pango-layout.h"
#include "types.h"
#include "utils.h"
#include <glycin-2/glycin.h>
#include <iostream>
#include <variant>
#include <vector>

#define PLAYER_UI_PATH "/org/riplay/data/ui/player.ui"

PlayerInstance::PlayerInstance(AppState *state) {
  this->state = state;
  artis_label_format = "%s - %s"; // format for artist - album label
  properties_format =
      " %d kbps - %d sec (%d:%02d) - %d Hz"; // format year label
  return;
}

PlayerInstance::~PlayerInstance() {}

void PlayerInstance::close() {
  if (win && media_stream) {
    media_stream->set_playing(false);
    win->close();
  }
}

void PlayerInstance::show() {
  auto builder = load_builder(PLAYER_UI_PATH);
  metadata = state->get_song()->get_metadata();

  // media_stream & media_controls
  media_stream =
      Gtk::MediaFile::create_for_filename(state->get_song()->get_filepath());
  media_stream->set_playing(true);
  media_controls = builder->get_object<Gtk::MediaControls>("audio_controls");
  media_controls->set_media_stream(media_stream);

  title_label = builder->get_object<Gtk::Label>("title_label");
  title_label->set_text(metadata->title);
  auto properties_label = builder->get_object<Gtk::Label>("year_label");
  properties_label->set_text(
      Glib::ustring::sprintf(properties_format, metadata->properties->bitrate,
                             metadata->properties->length,
                             (int)metadata->properties->length / MIN_IN_SECS,
                             (int)metadata->properties->length % MIN_IN_SECS,
                             metadata->properties->samplerate));
  auto artist_album_label =
      builder->get_object<Gtk::Label>("artist_album_label");
  artist_album_label->set_text(Glib::ustring::sprintf(
      artis_label_format, metadata->artist, metadata->album));

  auto albumart_content = builder->get_object<Gtk::Box>("album_lyrics_cont");

  if (metadata->raw_albumart) {
    auto loader = Gdk::PixbufLoader::create();
    loader->write(metadata->raw_albumart, metadata->raw_albumart_size);
    loader->close();
    auto pixbuf = loader->get_pixbuf();
    auto texture = Gdk::Texture::create_for_pixbuf(pixbuf);

    auto image = Gtk::make_managed<Gtk::Picture>(texture);
    image->set_vexpand(true);
    image->set_hexpand(true);
    albumart_content->append(*image);
  }

  // ui stuff
  win = builder->get_object<Gtk::Window>("player_window");

  state->get_app()->add_window(*win);
  win->show();
}

// static void draw_callback(GtkDrawingArea *area, cairo_t *cr, int width,
//                           int height, gpointer user_data) {
//   AppData *app = (AppData *)user_data;
//   GdkRGBA color;
//
//   cairo_set_antialias(cr, CAIRO_ANTIALIAS_BEST);
//   // Set drawing properties
//   cairo_set_source_rgb(cr, 1, 1, 1); // White color
//   cairo_set_line_width(cr, 5);       // Line width
//   cairo_paint(cr);
//
//   if (app->audio_data && app->data_size > 0) {
//     // Onda de audio en verde
//     cairo_set_source_rgb(cr, 0, 1, 0);
//     cairo_set_line_width(cr, 1.5);
//     cairo_move_to(cr, 0, (float)height / 2);
//
//     for (int i = 0; i < width; i++) {
//       int index = (i * app->data_size) / width;
//       float sample = app->audio_data[index];
//       float y = ((float)height / 2) + (sample * ((float)height / 2));
//       cairo_line_to(cr, i, y);
//     }
//     cairo_stroke(cr);
//   }
//   gtk_widget_get_color(GTK_WIDGET(area), &color);
//   gdk_cairo_set_source_rgba(cr, &color);
//   cairo_fill(cr);
// }
//
// static void update_visualizer(AppData *app) {
//   // Aquí iría la lógica para obtener los datos de audio actuales
//   // Esto es un ejemplo simplificado
//   if (app->media_stream && gtk_media_stream_get_playing(app->media_stream)) {
//     // Simular datos de audio (en una implementación real usarías GStreamer)
//     if (app->audio_data)
//       g_free(app->audio_data);
//     app->data_size = 1000;
//     app->audio_data = g_new(float, app->data_size);
//
//     for (int i = 0; i < app->data_size; i++) {
//       app->audio_data[i] = sin(i * 0.1) * 0.5;
//     }
//
//     gtk_widget_queue_draw(GTK_WIDGET(app->home->drawing_area));
//   } else {
//     if (app->audio_data)
//       g_free(app->audio_data);
//     app->audio_data = NULL;
//     gtk_widget_queue_draw(GTK_WIDGET(app->home->drawing_area));
//   }
// }
//
// static gboolean on_timeout_playing(gpointer user_data) {
//   AppData *app = (AppData *)user_data;
//   update_visualizer(app);
//   return G_SOURCE_CONTINUE;
// }
//
// int load_player_window(AppData *app_data) {
//   // load_actions(app_data);
//
//   unsigned long size;
//   unsigned char *img_data = extractAlbumArt(app_data->filename, &size);
//
//   GtkBuilder *b = load_builder("/org/riplay/data/ui/player.ui");
//
//   app_data->player->win =
//       GTK_WINDOW(gtk_builder_get_object(b, "player_window"));
//   printf("flaaaaaaaaaaaag -  entering to load_player_window %s\n",
//          app_data->filename);
//   auto lyrics = extractLyrics(app_data->filename);
//
//   if (std::holds_alternative<std::string>(lyrics)) {
//     app_data->lyric_props = std::vector<LyricProp>({});
//     app_data->lyrics =
//         parser_lyrics(std::get<std::string>(lyrics), app_data->lyric_props);
//   }
//
//   if (app_data->metadata && app_data->filename) {
//     app_data->media_stream =
//         gtk_media_file_new_for_filename(app_data->filename);
//     // gtk_media_stream_set_playing(app_data->media_stream, true);
//     gtk_media_stream_get_timestamp(app_data->media_stream);
//
//     const char *artis_label_format =
//         "%s - %s"; // format for artist - album label
//     const char *properties_format =
//         " %d kbps - %d sec (%d:%02d) - %d Hz"; // format year label
//
//     // get the labels from the builder
//     GtkWidget *title_label =
//         GTK_WIDGET(gtk_builder_get_object(b, "title_label"));
//     GtkWidget *properties_label =
//         GTK_WIDGET(gtk_builder_get_object(b, "year_label"));
//     GtkWidget *artis_album_label =
//         GTK_WIDGET(gtk_builder_get_object(b, "artis_album_label"));
//     app_data->media_controls =
//         GTK_WIDGET(gtk_builder_get_object(b, "audio_controls"));
//
//     // set strings to labels
//     gtk_label_set_ellipsize(GTK_LABEL(title_label), PANGO_ELLIPSIZE_END);
//     gtk_label_set_ellipsize(GTK_LABEL(properties_label),
//     PANGO_ELLIPSIZE_END);
//     gtk_label_set_ellipsize(GTK_LABEL(artis_album_label),
//     PANGO_ELLIPSIZE_END);
//
//     gtk_label_set_label(GTK_LABEL(title_label), app_data->metadata->title);
//     gtk_label_set_label(
//         GTK_LABEL(properties_label),
//         g_strdup_printf(
//             properties_format, app_data->metadata->properties->bitrate,
//             app_data->metadata->properties->length,
//             (int)app_data->metadata->properties->length / MIN_IN_SECS,
//             (int)app_data->metadata->properties->length % MIN_IN_SECS,
//             app_data->metadata->properties->samplerate));
//     gtk_label_set_label(GTK_LABEL(artis_album_label),
//                         g_strdup_printf(artis_label_format,
//                                         app_data->metadata->artist,
//                                         app_data->metadata->album));
//
//     gtk_media_controls_set_media_stream(
//         GTK_MEDIA_CONTROLS(app_data->media_controls),
//         app_data->media_stream);
//
//     GtkBox *albumart_content =
//         GTK_BOX(gtk_builder_get_object(b, "album_lyrics_cont"));
//
//     if (img_data) {
//       GdkPixbufLoader *loader = gdk_pixbuf_loader_new();
//       gdk_pixbuf_loader_write(loader, img_data, size, NULL);
//       gdk_pixbuf_loader_close(loader, NULL);
//       GdkPixbuf *pixbuf = gdk_pixbuf_loader_get_pixbuf(loader);
//       GdkTexture *texture = gdk_texture_new_for_pixbuf(pixbuf);
//
//       // Create a GTK image widget
//       GtkWidget *image =
//       gtk_picture_new_for_paintable(GDK_PAINTABLE(texture));
//       gtk_widget_set_vexpand(image, TRUE);
//       gtk_widget_set_hexpand(image, TRUE);
//       gtk_box_append(albumart_content, image);
//
//       g_object_unref(texture);
//       g_object_unref(loader);
//     }
//     app_data->lyrics_label = gtk_label_new("No lyrics");
//     if (!app_data->lyrics.empty()) {
//       gtk_label_set_label(GTK_LABEL(app_data->lyrics_label),
//                           app_data->lyrics[0].lyric.c_str());
//       start_lyrics_display(app_data->lyrics, app_data->media_stream,
//                            GTK_LABEL(app_data->lyrics_label));
//       if (!app_data->lyric_props.empty()) {
//         for (auto x : app_data->lyric_props) {
//           std::cout << x.field << " : " << x.value << std::endl;
//         }
//       }
//     }
//     gtk_widget_set_vexpand(app_data->lyrics_label, TRUE);
//     gtk_widget_set_hexpand(app_data->lyrics_label, TRUE);
//     gtk_label_set_wrap(GTK_LABEL(app_data->lyrics_label), true);
//     gtk_widget_set_margin_bottom(app_data->lyrics_label, 20);
//     gtk_widget_set_margin_top(app_data->lyrics_label, 20);
//     gtk_widget_set_margin_start(app_data->lyrics_label, 20);
//     gtk_widget_set_margin_end(app_data->lyrics_label, 20);
//     gtk_label_set_max_width_chars(GTK_LABEL(app_data->lyrics_label), 16);
//     gtk_label_set_width_chars(GTK_LABEL(app_data->lyrics_label), 8);
//     gtk_label_set_xalign(GTK_LABEL(app_data->lyrics_label),
//     GTK_ALIGN_CENTER); gtk_box_append(albumart_content,
//     app_data->lyrics_label);
//     // init visualizer
//     g_object_unref(b);
//
//     gtk_application_add_window(GTK_APPLICATION(app_data->app),
//                                GTK_WINDOW(app_data->player->win));
//     gtk_window_present(GTK_WINDOW(app_data->player->win));
//     return 0;
//   } else {
//     g_printerr("error loading player!");
//     return EXIT_FAILURE;
//   }
// }
