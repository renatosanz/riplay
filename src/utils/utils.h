#ifndef BUILDER_UTILS
#define BUILDER_UTILS
#include "gtk/gtk.h"
#include "types.h"

GtkBuilder *load_builder(const char *path);
void free_global_resources(AppData *app_data);
#endif // !BUILDER_UTILS
