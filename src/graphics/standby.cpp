#include "gdk/gdk.h"
#include "gtk/gtk.h"

static int position = 0;

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
gboolean on_timeout(gpointer user_data) {
  // Update animation position (resets after 500 frames)
  position = (position + 1) % 500;

  // Trigger redraw
  GtkDrawingArea *area = GTK_DRAWING_AREA(user_data);
  gtk_widget_queue_draw(GTK_WIDGET(area));

  return G_SOURCE_CONTINUE;
}
