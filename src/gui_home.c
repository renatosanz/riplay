/**
 * @file gui_home.c
 * @brief GUI window management and file dialogs
 *
 * This module handles the creation and management the
 * home window with standby animation and file dialogs.
 */

#include "file_history.h"
#include "gui.h"
#include "riplay.h"

// Static variables for animation control
guint timeout_id = 0;    // Timeout ID for animation cancellation
static int position = 0; // Position variable for animation drawing

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
int load_home_window(GApplication *app, GtkWidget **spectrum, GtkWindow **win) {
  // Load actions for UI buttons
  load_actions(app);

  // Load UI from resources
  GtkBuilder *builder = load_builder("/org/riplay/data/ui/default.ui");
  if (!builder) {
    g_critical("Failed to load home window UI");
    return -1;
  }

  // Get window from builder
  *win = GTK_WINDOW(gtk_builder_get_object(builder, "default_window"));
  if (!*win) {
    g_critical("Failed to get home window from builder");
    g_object_unref(builder);
    return -1;
  }

  // Set up standby animation
  *spectrum = GTK_WIDGET(gtk_builder_get_object(builder, "spectrum_default"));
  gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(*spectrum),
                                 draw_stand_by_function, NULL, NULL);

  // Start animation timer (32ms interval ≈ 30fps)
  timeout_id = g_timeout_add(32, on_timeout, *spectrum);

  // Clean up builder
  g_object_unref(builder);

  // Display window
  gtk_application_add_window(GTK_APPLICATION(app), *win);
  gtk_window_present(*win);

  return 0;
}

/**
 * @brief Draws standby animation (sine wave)
 *
 * Callback function for drawing the standby animation on the home window.
 *
 * @param area The drawing area widget
 * @param cr Cairo drawing context
 * @param width Drawing area width
 * @param height Drawing area height
 * @param data User data (unused)
 */
void draw_stand_by_function(GtkDrawingArea *area, cairo_t *cr, int width,
                            int height, gpointer data) {
  (void)data;
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
  gtk_widget_get_color(GTK_WIDGET(area), &color);
  gdk_cairo_set_source_rgba(cr, &color);
  cairo_fill(cr);
}

/**
 * @brief Animation timer callback
 *
 * Updates animation position and triggers redraw.
 *
 * @param user_data The drawing area widget
 * @return gboolean G_SOURCE_CONTINUE to keep timer running
 */
static gboolean on_timeout(gpointer user_data) {
  // Update animation position (resets after 500 frames)
  position = (position + 1) % 500;

  // Trigger redraw
  GtkDrawingArea *area = GTK_DRAWING_AREA(user_data);
  gtk_widget_queue_draw(GTK_WIDGET(area));

  return G_SOURCE_CONTINUE;
}

/**
 * @brief Handles file selection dialog response
 *
 * Callback for when user selects a file from the open dialog.
 *
 * @param source_object The file dialog
 * @param result Async result
 * @param user_data User data (unused)
 */
void file_dialog_response(GObject *source_object, GAsyncResult *result,
                          gpointer user_data) {
  (void)user_data;
  GtkFileDialog *dialog = GTK_FILE_DIALOG(source_object);
  GError *error = NULL;

  // Get selected file
  GFile *file = gtk_file_dialog_open_finish(dialog, result, &error);

  if (error) {
    g_printerr("File selection error: %s\n", error->message);
    g_error_free(error);
    return;
  }

  if (!file) {
    g_print("No file selected\n");
    return;
  }

  // Get file path
  char *filepath = g_file_get_path(file);
  if (!filepath) {
    g_object_unref(file);
    g_critical("Failed to get file path");
    return;
  }

  // Clean up animation if running
  if (timeout_id > 0) {
    g_source_remove(timeout_id);
    timeout_id = 0;
  }

  // add filepath to file history
  add_to_recent_files(filepath);

  // Start new playback session
  clean_new_on_playing(filepath);

  // Clean up
  g_free(filepath);
  g_object_unref(file);
}

/**
 * @brief Opens file selection dialog
 *
 * Creates and displays a file dialog for selecting audio files.
 *
 * @param action The action that triggered this (unused)
 * @param parameter Action parameter (unused)
 * @param app The application instance
 */
void open_new_file_dialog(GSimpleAction *action, GVariant *parameter,
                          GApplication *app) {
  (void)action;
  (void)parameter;

  GtkWindow *window = GTK_WINDOW(app);
  GtkFileDialog *dialog = gtk_file_dialog_new();

  // Configure dialog
  gtk_file_dialog_set_title(dialog, "Select a song");

  // Set audio file filter
  GtkFileFilter *filter = gtk_file_filter_new();
  gtk_file_filter_add_mime_type(filter, "audio/*");
  gtk_file_dialog_set_default_filter(dialog, filter);

  // Show dialog
  gtk_file_dialog_open(dialog, window, NULL, file_dialog_response, NULL);
}
