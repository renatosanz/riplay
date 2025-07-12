#ifndef GTK_UTILS_H
#define GTK_UTILS_H

#include "gtk/gtk.h"
#include <types.h>
int *open_single_window(GApplication *app, const char *filename,
                      GtkWidget **spectrum, GtkWindow **win,GtkWidget **player,
                      FileMetaData *metadata);
#endif
