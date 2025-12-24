#include "player.h"
#include "actions.h"
#include "gdkmm/pixbuf.h"
#include "gdkmm/texture.h"
#include "gio/gio.h"
#include "glib-object.h"
#include "glib.h"
#include "glibmm/ustring.h"
#include "gtkmm.h"
#include "gtkmm/button.h"
#include "gtkmm/label.h"
#include "gtkmm/mediacontrols.h"
#include "gtkmm/mediafile.h"
#include "gtkmm/object.h"
#include "gtkmm/picture.h"
#include "metadata/metadata.h"
#include "models/models.h"
#include "mpegfile.h"
#include "pango/pango-layout.h"
#include "pangomm/layout.h"
#include "sigc++/functors/mem_fun.h"
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
  date_format = "Released on %d";
  gender_format = "Gender: %s";
  artist_format = "Artist(s): %s";
  path_format = "Filepath: %s";
  return;
}

PlayerInstance::~PlayerInstance() {}

void PlayerInstance::close() {
  if (win && media_stream) {
    media_stream->set_playing(false);
    win->close();
    end_lyrics_thread();
    std::cout << "PlayerInstance closed!!\n";
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

  setup_labels(builder);
  setup_albumart(builder);
  setup_button_actions(builder);
  setup_metadata_side(builder);
  setup_lyrics(builder);
  // ui stuff
  win = builder->get_object<Gtk::Window>("player_window");

  state->add_window(*win);
  win->present();
}

void PlayerInstance::setup_metadata_side(Glib::RefPtr<Gtk::Builder> builder) {
  metadata_side = builder->get_object<Gtk::Box>("metadata_side");

  auto filepath_label = builder->get_object<Gtk::Label>("filepath_label");
  filepath_label->set_label(
      Glib::ustring::sprintf(path_format, state->get_song()->get_filepath()));
  filepath_label->set_tooltip_text(state->get_song()->get_filepath());
  auto filespecs_label = builder->get_object<Gtk::Label>("filespecs_label");
  filespecs_label->set_label(
      Glib::ustring::sprintf(properties_format, metadata->properties->bitrate,
                             metadata->properties->length,
                             (int)metadata->properties->length / MIN_IN_SECS,
                             (int)metadata->properties->length % MIN_IN_SECS,
                             metadata->properties->samplerate));
  auto gender_label = builder->get_object<Gtk::Label>("gender_label");
  gender_label->set_label(
      Glib::ustring::sprintf(gender_format, metadata->genre));
  auto date_label = builder->get_object<Gtk::Label>("date_label");
  date_label->set_label(Glib::ustring::sprintf(date_format, metadata->year));
  auto artist_label = builder->get_object<Gtk::Label>("artist_label");
  artist_label->set_label(
      Glib::ustring::sprintf(artist_format, metadata->artist));
}

void PlayerInstance::setup_lyrics(Glib::RefPtr<Gtk::Builder> builder) {
  auto sync_lyrics = state->get_song()->get_sync_lyrics();
  auto raw_lyrics = state->get_song()->get_raw_lyrics();
  auto lyrics_props = state->get_song()->get_lyrics_props();

  if (!sync_lyrics.empty()) {
    lyrics_label->set_label(sync_lyrics[0].lyric.c_str());
    // start_lyrics_display(app_data->lyrics, app_data->media_stream,
    //                      GTK_LABEL(app_data->lyrics_label));
    start_lyrics_display(sync_lyrics, media_stream, lyrics_label);
    if (!lyrics_props.empty()) {
      for (auto x : lyrics_props) {
        std::cout << x.field << " : " << x.value << std::endl;
      }
    }
  } else if (raw_lyrics.length() > 0) {
    lyrics_label->set_label(raw_lyrics);
  }
}

void PlayerInstance::setup_button_actions(Glib::RefPtr<Gtk::Builder> builder) {
  auto lyrics_btn = builder->get_object<Gtk::Button>("lyrics_btn");
  lyrics_btn->signal_clicked().connect([this]() {
    lyrics_visible = !lyrics_visible;
    lyrics_label->set_visible(lyrics_visible);
  });

  auto metadata_side_btn =
      builder->get_object<Gtk::Button>("metadata_side_btn");
  metadata_side_btn->signal_clicked().connect([this]() {
    metadata_side_visible = !metadata_side_visible;
    metadata_side->set_visible(metadata_side_visible);
  });
}

void PlayerInstance::setup_albumart(Glib::RefPtr<Gtk::Builder> builder) {
  auto albumart_content = builder->get_object<Gtk::Box>("album_lyrics_cont");
  lyrics_label = builder->get_object<Gtk::Label>("lyrics_label");

  if (metadata->raw_albumart) {
    auto loader = Gdk::PixbufLoader::create();
    loader->write(metadata->raw_albumart, metadata->raw_albumart_size);
    loader->close();
    auto pixbuf = loader->get_pixbuf();
    auto texture = Gdk::Texture::create_for_pixbuf(pixbuf);

    albumart_picture = builder->get_object<Gtk::Picture>("albumart_picture");
    albumart_picture->set_paintable(texture);
    albumart_picture->set_vexpand(true);
    albumart_picture->set_hexpand(true);
  }
}

void PlayerInstance::setup_labels(Glib::RefPtr<Gtk::Builder> builder) {
  title_label = builder->get_object<Gtk::Label>("title_label");
  title_label->set_text(metadata->title);
  title_label->set_ellipsize(Pango::EllipsizeMode::END);
  auto properties_label = builder->get_object<Gtk::Label>("year_label");
  properties_label->set_text(
      Glib::ustring::sprintf(properties_format, metadata->properties->bitrate,
                             metadata->properties->length,
                             (int)metadata->properties->length / MIN_IN_SECS,
                             (int)metadata->properties->length % MIN_IN_SECS,
                             metadata->properties->samplerate));
  properties_label->set_ellipsize(Pango::EllipsizeMode::END);
  auto artist_album_label =
      builder->get_object<Gtk::Label>("artist_album_label");
  artist_album_label->set_ellipsize(Pango::EllipsizeMode::END);
  artist_album_label->set_text(Glib::ustring::sprintf(
      artis_label_format, metadata->artist, metadata->album));
}

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
//         parser_lyrics(std::get<std::string>(lyrics),
//         app_data->lyric_props);
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
//
//       g_object_unref(texture);
//       g_object_unref(loader);
//     }
//     gtk_widget_set_vexpand(app_data->lyrics_label, TRUE);
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
