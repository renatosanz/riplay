#ifndef GTK_UTILS_H
#define GTK_UTILS_H
#define MIN_IN_SECS 60


#include <gtk/gtk.h>
#include "types.h"

extern guint timeout_id;

// actions
void load_actions(GApplication *app);

// windows
GtkBuilder *load_builder(const char *path);
int load_home_window(GApplication *app, GtkWidget **spectrum, GtkWindow **win);
int load_player_window(GApplication *app, const char *filename,
                       GtkWidget **spectrum, GtkWindow **win,
                       GtkWidget **player, GtkMediaStream **stream,
                       FileMetaData *metadata);

// action callbacks
void open_new_file_dialog(GSimpleAction *action, GVariant *parameter,
                          GApplication *app);
void open_visuals_menu(GSimpleAction *action, GVariant *parameter,
                       GApplication *app);
void open_recent_files(GSimpleAction *action, GVariant *parameter,
                       GApplication *app);
void open_equalizer(GSimpleAction *action, GVariant *parameter,
                    GApplication *app);
void toggle_enable_equalizer(GSimpleAction *action, GVariant *parameter,
                      GApplication *app);
void close_equalizer(GSimpleAction *action, GVariant *parameter,
                      GApplication *app);

// animations
void draw_stand_by_function(GtkDrawingArea *area, cairo_t *cr, int width,
                            int height, gpointer data);
static gboolean on_timeout(gpointer user_data);

#endif
