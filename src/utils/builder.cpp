#include "gtkmm/builder.h"
#include "glibmm/error.h"
#include "glibmm/refptr.h"

Glib::RefPtr<Gtk::Builder> load_builder(const char *path) {
  auto builder = Gtk::Builder::create();
  if (!builder->add_from_resource(path)) {
    throw Glib::Error();
  }
  return builder;
}
