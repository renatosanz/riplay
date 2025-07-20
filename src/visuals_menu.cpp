#include "gui.h"

static void close_visuals_window(GtkButton *button, gpointer user_data);

void open_visuals_menu(GSimpleAction *action, GVariant *parameter,
                       GApplication *app)
{
  GtkBuilder *b = load_builder("/org/riplay/data/ui/visuals_menu.ui");
  GtkWindow *visuals_win =
      GTK_WINDOW(gtk_builder_get_object(b, "visuals_menu"));

  GtkButton *back_button = GTK_BUTTON(gtk_builder_get_object(b, "back_btn"));
  if (!back_button)
  {
    g_critical("Failed to get recent files window");
    g_object_unref(b);
    return;
  }
  g_signal_connect(back_button, "clicked", G_CALLBACK(close_visuals_window), visuals_win);

  gtk_application_add_window(GTK_APPLICATION(app), GTK_WINDOW(visuals_win));
  gtk_window_present(GTK_WINDOW(visuals_win));
}

static void close_visuals_window(GtkButton *button, gpointer user_data)
{
  GtkWindow *parent_window = GTK_WINDOW(user_data);
  gtk_window_close(parent_window);
}