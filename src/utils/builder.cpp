
#include "gtk/gtk.h"

/**
 * @brief Loads a GtkBuilder from a resource path
 *
 * Creates a new GtkBuilder instance and loads the UI definition from
 * the specified resource path. The resource should be compiled into
 * the application's GResource bundle.
 *
 * @param path The resource path to the UI definition file
 * @return GtkBuilder* A new builder instance with the loaded UI
 *
 * @note The caller is responsible for unreferencing the returned builder
 */
GtkBuilder *load_builder(const char *path) {
  GtkBuilder *builder = gtk_builder_new();
  if (!gtk_builder_add_from_resource(builder, path, NULL)) {
    g_warning("Failed to load UI resource: %s", path);
    g_object_unref(builder);
    return NULL;
  }
  return builder;
}
