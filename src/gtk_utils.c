#include "cairo.h"
#include "gio/gio.h"
#include "glib-object.h"
#include "glib.h"
#include "gtk/gtk.h"
#include "gtk/gtkshortcut.h"
#include "pango/pango-layout.h"
#include "riplay.h"
#include "types.h"
#include <math.h>
#include <stdlib.h>
#include <taglib/tag_c.h>

#define MIN_IN_SECS 60

static guint timeout_id = 0;
static int position = 0;

static void open_recent_files(GSimpleAction *action, GVariant *parameter,
                              GApplication *app);
static gboolean on_timeout(gpointer user_data);
void load_actions(GApplication *app);
static void open_visuals_menu(GSimpleAction *action, GVariant *parameter,
                              GApplication *app);
static void open_new_file_dialog(GSimpleAction *action, GVariant *parameter,
                                 GApplication *app);

void file_dialog_response(GObject *source_object, GAsyncResult *result,
                          gpointer user_data) {
  GtkFileDialog *dialog = GTK_FILE_DIALOG(source_object);
  GFile *file = NULL;
  GError *error = NULL;

  // Obtener el archivo seleccionado
  file = gtk_file_dialog_open_finish(dialog, result, &error);

  if (error != NULL) {
    // Hubo un error
    g_printerr("Error seleccionando archivo: %s\n", error->message);
    g_error_free(error);
    return;
  }

  if (file != NULL) {
    // Obtener la ruta del archivo como string
    char *filepath = g_file_get_path(file);

    if (filepath != NULL) {
      // Imprimir la ruta del archivo
      g_print("Archivo seleccionado: %s\n", filepath);

      if (timeout_id > 0) {
        g_source_remove(timeout_id);
        timeout_id = 0;
      }

      clean_new_on_playing(filepath);

      // Liberar memoria
      g_free(filepath);
    }

    // Liberar el objeto GFile
    g_object_unref(file);
  } else {
    g_print("No se seleccionó ningún archivo\n");
  }
}

static GtkBuilder *loadBuilder(const char *path) {
  GtkBuilder *builder = gtk_builder_new();
  gtk_builder_add_from_resource(builder, path, NULL);
  return builder;
}

static void draw_stand_by_function(GtkDrawingArea *area, cairo_t *cr, int width,
                                   int height, gpointer data) {
  (void)data;
  GdkRGBA color;

  // cairo_arc(cr, width / 2.0, height / 2.0, MIN(width, height) / 2.0, 0,
  //           1 * G_PI);

  cairo_set_source_rgb(cr, 1, 1, 1);
  cairo_set_line_width(cr, 5);
  cairo_move_to(cr, 0, height / 2.0);
  float frequency = 1.0 / 10.0;
  float mid_height = height / 2.0;

  for (int i = 1; i <= width; i++) {
    float y_offset = 20 * sin((position + i) * frequency);
    cairo_line_to(cr, i, mid_height + y_offset);
  }
  cairo_stroke(cr);

  gtk_widget_get_color(GTK_WIDGET(area), &color);
  gdk_cairo_set_source_rgba(cr, &color);

  cairo_fill(cr);
}

int load_player_window(GApplication *app, const char *filename,
                       GtkWidget **spectrum, GtkWindow **win,
                       GtkWidget **player, GtkMediaStream **stream,
                       FileMetaData *metadata) {
  load_actions(app);

  GtkBuilder *b = loadBuilder("/org/riplay/data/ui/player.ui");
  *win = GTK_WINDOW(gtk_builder_get_object(b, "player_window"));

  if (metadata && filename) {
    *stream = gtk_media_file_new_for_filename(filename);
    // labels
    const char *artis_label_format =
        "%s - %s"; // format for artist - album label
    const char *propieties_format =
        " %d kbps - %d sec (%d:%02d) - %d Hz"; // format year label
    GtkWidget *title_label;                    // title song label
    GtkWidget *artis_album_label;              // label for artist - album
    GtkWidget *propieties_label;               // label for song year

    // get the labels from the builder
    title_label = GTK_WIDGET(gtk_builder_get_object(b, "title_label"));
    propieties_label = GTK_WIDGET(gtk_builder_get_object(b, "year_label"));
    artis_album_label =
        GTK_WIDGET(gtk_builder_get_object(b, "artis_album_label"));
    *player = GTK_WIDGET(gtk_builder_get_object(b, "audio_controls"));

    // set strings to labels
    gtk_label_set_ellipsize(GTK_LABEL(title_label), PANGO_ELLIPSIZE_END);
    gtk_label_set_ellipsize(GTK_LABEL(propieties_label), PANGO_ELLIPSIZE_END);
    gtk_label_set_ellipsize(GTK_LABEL(artis_album_label), PANGO_ELLIPSIZE_END);

    gtk_label_set_label(GTK_LABEL(title_label), metadata->title);
    gtk_label_set_label(
        GTK_LABEL(propieties_label),
        g_strdup_printf(propieties_format, metadata->propieties->bitrate,
                        metadata->propieties->length,
                        (int)metadata->propieties->length / MIN_IN_SECS,
                        (int)metadata->propieties->length % MIN_IN_SECS,
                        metadata->propieties->samplerate));
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
  } else {
    g_printerr("error loading player!");
    return EXIT_FAILURE;
  }
}

int load_default_window(GApplication *app, const char *filename,
                        GtkWidget **spectrum, GtkWindow **win,
                        GtkWidget **player, FileMetaData *metadata) {

  load_actions(app);

  GtkBuilder *b = loadBuilder("/org/riplay/data/ui/default.ui");
  *win = GTK_WINDOW(gtk_builder_get_object(b, "default_window"));

  *spectrum = GTK_WIDGET(gtk_builder_get_object(b, "spectrum_default"));
  gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(*spectrum),
                                 draw_stand_by_function, NULL, NULL);

  timeout_id =
      g_timeout_add(32, on_timeout, *spectrum); // draw stand_by animation

  g_object_unref(b);

  gtk_application_add_window(GTK_APPLICATION(app), GTK_WINDOW(*win));
  gtk_window_present(GTK_WINDOW(*win));
  return 0;
}

void openMultiWindow(void) {}

void load_actions(GApplication *app) {
  // load actions
  GSimpleAction *recents_action_obj = g_simple_action_new("open-recents", NULL);
  GSimpleAction *visuals_action_obj =
      g_simple_action_new("change-visuals", NULL);
  GSimpleAction *open_new_file_action_obj =
      g_simple_action_new("open-new-file", NULL);

  g_signal_connect(recents_action_obj, "activate",
                   G_CALLBACK(open_recent_files), app);
  g_signal_connect(visuals_action_obj, "activate",
                   G_CALLBACK(open_visuals_menu), app);
  g_signal_connect(open_new_file_action_obj, "activate",
                   G_CALLBACK(open_new_file_dialog), app);

  GActionMap *action_map = G_ACTION_MAP(app);
  g_action_map_add_action(action_map, G_ACTION(recents_action_obj));
  g_action_map_add_action(action_map, G_ACTION(visuals_action_obj));
  g_action_map_add_action(action_map, G_ACTION(open_new_file_action_obj));
}

static void open_recent_files(GSimpleAction *action, GVariant *parameter,
                              GApplication *app) {

  GtkBuilder *b = loadBuilder("/org/riplay/data/ui/recents.ui");
  GtkWindow *recent_win = GTK_WINDOW(gtk_builder_get_object(b, "recents"));

  // look for recent files and show them (max 5 files)
  GtkBox *recent_files_box =
      GTK_BOX(gtk_builder_get_object(b, "recent_files_box"));
  const char *label_format = "file %d"; // format for recent files

  for (int i = 0; i < 5; i++) {
    GtkWidget *button =
        gtk_button_new_with_label(g_strdup_printf(label_format, i));
    gtk_box_append(GTK_BOX(recent_files_box), button);

    // Connect signals here if needed
    // g_signal_connect(button, "clicked", G_CALLBACK(on_file_clicked),
    // GINT_TO_POINTER(i));
  }

  gtk_application_add_window(GTK_APPLICATION(app), GTK_WINDOW(recent_win));
  gtk_window_present(GTK_WINDOW(recent_win));
}

static void open_visuals_menu(GSimpleAction *action, GVariant *parameter,
                              GApplication *app) {
  GtkBuilder *b = loadBuilder("/org/riplay/data/ui/visuals_menu.ui");
  GtkWindow *visuals_win =
      GTK_WINDOW(gtk_builder_get_object(b, "visuals_menu"));

  gtk_application_add_window(GTK_APPLICATION(app), GTK_WINDOW(visuals_win));
  gtk_window_present(GTK_WINDOW(visuals_win));
}

static void open_new_file_dialog(GSimpleAction *action, GVariant *parameter,
                                 GApplication *app) {
  GtkWindow *window = GTK_WINDOW(app);
  GtkFileDialog *dialog;
  dialog = GTK_FILE_DIALOG(gtk_file_dialog_new());
  gtk_file_dialog_set_title(dialog, "Seleccionar archivo");

  GtkFileFilter *filter = gtk_file_filter_new();
  gtk_file_filter_add_mime_type(filter, "audio/*");
  gtk_file_dialog_set_default_filter(dialog, filter);

  gtk_file_dialog_open(dialog, window, NULL, file_dialog_response, NULL);
}

static gboolean on_timeout(gpointer user_data) {
  position += 1;        // Mover el círculo a la derecha
  if (position > 500) { // Reiniciar la posición si sale de la pantalla
    position = 0;
  }

  GtkDrawingArea *area = GTK_DRAWING_AREA(user_data);
  gtk_widget_queue_draw(GTK_WIDGET(area)); // Solicitar redibujo del área

  return G_SOURCE_CONTINUE; // Continuar el temporizador
}
