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
#include "glib-object.h"
#include "gtk/gtk.h"
#include "models/models.h"
#include "utils.h"

HomeInstance::HomeInstance(AppState *state) {
  printf("Creating HomeInstance()...\n");
  this->app_state = state;

  GtkBuilder *builder = load_builder("/org/riplay/data/ui/home.ui");
  if (!builder) {
    g_critical("Failed to load home window UI");
  }

  win = GTK_WINDOW(gtk_builder_get_object(builder, "default_window"));
  // set up standby animation
  drawing_area =
      GTK_DRAWING_AREA(gtk_builder_get_object(builder, "spectrum_default"));
  gtk_drawing_area_set_draw_func(
      drawing_area,
      +[](GtkDrawingArea *area, cairo_t *cr, int width, int height,
          gpointer data) -> void {
        auto *self = static_cast<HomeInstance *>(data);
        self->draw_stand_by_function(cr, width, height);
      },
      this, // Pass the instance pointer
      NULL);

  // Start animation timer (32ms interval ≈ 30fps)
  timeout_id = g_timeout_add(32, on_timeout, this);

  g_object_unref(builder);
}

int HomeInstance::show() {
  gtk_application_add_window(GTK_APPLICATION(app_state->get_app()), win);
  gtk_window_present(win);
  return 0;
}

HomeInstance::~HomeInstance() {
  if (timeout_id > 0) {
    g_source_remove(timeout_id);
  }
}

gboolean HomeInstance::on_timeout(gpointer user_data) {
  HomeInstance *self = static_cast<HomeInstance *>(user_data);

  // Update animation position (resets after 500 frames)
  self->position = (self->position + 1) % 500;

  // Trigger redraw
  GtkDrawingArea *area = GTK_DRAWING_AREA(self->drawing_area);
  gtk_widget_queue_draw(GTK_WIDGET(area));

  return G_SOURCE_CONTINUE;
}

void HomeInstance::draw_stand_by_function(cairo_t *cr, int width, int height) {
  GdkRGBA color;

  const float frequency = 1.0f / 10.0f;   // Wave frequency
  const float mid_height = height / 2.0f; // Vertical center

  // Set drawing properties
  cairo_set_source_rgb(cr, 1, 1, 1); // White color
  cairo_set_line_width(cr, 5);       // Line width
  cairo_move_to(cr, 0, mid_height);  // Start at left center

  // Draw sine wave
  for (int i = 1; i <= width; i++) {
    float y_offset = 20 * sin((position + i) * frequency);
    cairo_line_to(cr, i, mid_height + y_offset);
  }

  // Render the path
  cairo_stroke(cr);

  // Fill with widget's background color
  gtk_widget_get_color(GTK_WIDGET(drawing_area), &color);
  gdk_cairo_set_source_rgba(cr, &color);
  cairo_fill(cr);
}

// int show_home_view(AppData *app_data) {
//   g_return_val_if_fail(app_data->app != NULL && app_data != NULL, -1);
//
//   // load ui from resources
//   GtkBuilder *builder = load_builder("/org/riplay/data/ui/home.ui");
//   if (!builder) {
//     g_critical("Failed to load home window UI");
//     return -1;
//   }
//
//   // get window from builder
//   app_data->home->win =
//       GTK_WINDOW(gtk_builder_get_object(builder, "default_window"));
//   if (!app_data->home->win) {
//     g_critical("Failed to get home window from builder");
//     g_object_unref(builder);
//     return -1;
//   }
//
//   // set up standby animation
//   app_data->home->drawing_area =
//       GTK_DRAWING_AREA(gtk_builder_get_object(builder, "spectrum_default"));
//   gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(app_data->home->drawing_area),
//                                  draw_stand_by_function, NULL, NULL);
//
//   // Start animation timer (32ms interval ≈ 30fps)
//   app_data->home->timeout_id =
//       g_timeout_add(32, on_timeout, app_data->home->drawing_area);
//
//   // clean up builder
//   g_object_unref(builder);
//
//   // display window
//   gtk_application_add_window(GTK_APPLICATION(app_data->app),
//                              app_data->home->win);
//   gtk_window_present(app_data->home->win);
//
//   return 0;
// }
//
// void file_dialog_response(GObject *source_object, GAsyncResult *result,
//                           AppData *app_data) {
//   GtkFileDialog *dialog = GTK_FILE_DIALOG(source_object);
//   GError *error = NULL;
//
//   // Get selected file
//   GFile *file = gtk_file_dialog_open_finish(dialog, result, &error);
//
//   if (error) {
//     g_printerr("File selection error: %s\n", error->message);
//     g_error_free(error);
//     return;
//   }
//
//   if (!file) {
//     g_print("No file selected\n");
//     return;
//   }
//
//   // Get file path
//   char *filepath = g_file_get_path(file);
//   if (!filepath) {
//     g_object_unref(file);
//     g_critical("Failed to get file path");
//     return;
//   }
//
//   // add filepath to file history
//   add_to_recent_files(filepath);
//
//   // Start new playback session
//   clean_new_on_playing(filepath);
//
//   // Clean up
//   g_free(filepath);
//   g_object_unref(file);
// }
//
// /**
//  * @brief Opens file selection dialog
//  *
//  * Creates and displays a file dialog for selecting audio files.
//  *
//  * @param action The action that triggered this (unused)
//  * @param parameter Action parameter (unused)
//  * @param app The application instance
//  */
// void open_new_file_dialog(GSimpleAction *action, GVariant *parameter,
//                           AppData *app_data) {
//   (void)action;
//   (void)parameter;
//
//   GtkWindow *window = GTK_WINDOW(app_data->home->win);
//   GtkFileDialog *dialog = gtk_file_dialog_new();
//
//   // Configure dialog
//   gtk_file_dialog_set_title(dialog, "Select a song");
//
//   // Set audio file filter
//   GtkFileFilter *filter = gtk_file_filter_new();
//   gtk_file_filter_add_mime_type(filter, "audio/*");
//   gtk_file_dialog_set_default_filter(dialog, filter);
//
//   // Show dialog
//   gtk_file_dialog_open(dialog, window, NULL,
//                        GAsyncReadyCallback(file_dialog_response), app_data);
// }
