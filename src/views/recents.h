#ifndef RECENTS
#define RECENTS
#include "gtk/gtk.h"
#include "types.h"

void close_recents_window(GtkButton *button, GtkWindow *win);
void open_recent_files(GSimpleAction *action, GVariant *parameter,
                       AppData *app_data);
#endif // !RECENTS
