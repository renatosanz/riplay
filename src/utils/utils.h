#ifndef BUILDER_UTILS
#define BUILDER_UTILS
#include "gtk/gtk.h"
#include "gtkmm/builder.h"
#include "types.h"
#include <variant>

Glib::RefPtr<Gtk::Builder> load_builder(const char *path);
void free_global_resources(AppData *app_data);
#endif // !BUILDER_UTILS
