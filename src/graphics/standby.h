#ifndef STANDBY
#define STANDBY
#include "gtk/gtk.h"

void draw_stand_by_function(GtkDrawingArea *area, cairo_t *cr, int width,
                            int height, gpointer data);

gboolean on_timeout(gpointer user_data);
#endif // !STANDBY
