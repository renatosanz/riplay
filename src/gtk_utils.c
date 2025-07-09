#include "gtk/gtk.h"
#include <stdio.h>

static GtkBuilder *loadBuilder(const char *path);

GtkWindow *openSingleWindow(GApplication *app, const char *filename) {
  GtkBuilder *b = loadBuilder("/org/riplay/data/ui/player.ui");
  GtkWindow *win = GTK_WINDOW(gtk_builder_get_object(b, "player"));
  gtk_window_set_default_size(GTK_WINDOW(win), 500, 300);

  if (filename) {
    GtkLabel *label = GTK_LABEL(gtk_builder_get_object(b, "audio_label"));
    gtk_label_set_label(label, filename);
    // hide spectrum zone
    gtk_widget_set_visible(
        GTK_WIDGET(gtk_builder_get_object(b, "init_options_box")), FALSE);
  } else {
    // hide spectrum zone
    gtk_widget_set_visible(GTK_WIDGET(gtk_builder_get_object(b, "spectrum")),
                           FALSE);

    // look for recent files and show them (max 5 files)
    GtkBox *recent_files_box =
        GTK_BOX(gtk_builder_get_object(b, "recent_files_box"));

    const char *label_format = "file %d";

    for (int i = 0; i < 2; i++) {
      GtkWidget *button = gtk_button_new_with_label(NULL);
      gtk_button_set_label(GTK_BUTTON(button),
                           g_strdup_printf(label_format, i));
      gtk_box_append(GTK_BOX(recent_files_box), button);

      // Connect signals here if needed
      // g_signal_connect(button, "clicked", G_CALLBACK(on_file_clicked),
      // GINT_TO_POINTER(i));
    }
  }
  g_object_unref(b);

  gtk_application_add_window(GTK_APPLICATION(app), GTK_WINDOW(win));
  gtk_window_present(GTK_WINDOW(win));
  return win;
}

void openMultiWindow(void) {}

GtkBuilder *loadBuilder(const char *path) {
  GtkBuilder *builder = gtk_builder_new();
  gtk_builder_add_from_resource(builder, path, NULL);
  return builder;
}
