#ifndef GTK_UTILS_H
#define GTK_UTILS_H

#include "gtk/gtk.h"
#include <types.h>
int load_default_window(GApplication *app, const char *filename,
                      GtkWidget **spectrum, GtkWindow **win,GtkWidget **player,
                      FileMetaData *metadata);
int load_player_window(GApplication *app, const char *filename,
                       GtkWidget **spectrum, GtkWindow **win,
                       GtkWidget **player, GtkMediaStream **stream,
                       FileMetaData *metadata);
void close_all_windows(GtkApplication *app, AppData *app_data);
#endif
