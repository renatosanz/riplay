#include "file_history.h"
#include "gio/gio.h"
#include "giomm/file.h"
#include "glib.h"
#include "glibmm/refptr.h"
#include "gtk/gtk.h"
#include "gtkmm/button.h"
#include "gtkmm/label.h"
#include "gtkmm/object.h"
#include "gtkmm/widget.h"
#include "gtkmm/window.h"
#include "models/models.h"
#include "player.h"
#include "sigc++/adaptors/bind.h"
#include "sigc++/functors/mem_fun.h"
#include "types.h"
#include "utils.h"
#include <cstdio>
#include <cstring>
#include <sys/ucontext.h>

#define MAX_RECENT_FILES 5
#define RECENTS_UI_PATH "/org/riplay/data/ui/recents.ui"

RecentsInstance::RecentsInstance(AppState *state) {
  printf("Creating RecentsInstance()...");
  this->state = state;

  // g_signal_connect(back_button, "clicked", G_CALLBACK(close_recents_window),
  //                  this);
}

RecentsInstance::~RecentsInstance() {}

void RecentsInstance::show(const Glib::VariantBase &parameter) {
  auto builder = load_builder(RECENTS_UI_PATH);

  win = builder->get_object<Gtk::Window>("recents");
  recent_files_box = builder->get_object<Gtk::Box>("recent_files_box");
  auto back_button = builder->get_object<Gtk::Button>("back_btn");
  back_button->signal_clicked().connect(
      sigc::mem_fun(*this, &RecentsInstance::close));

  auto recent_files = get_recent_files_list(MAX_RECENT_FILES);
  if (recent_files.empty()) {
    // add "no recent files" label if empty
    auto label = Gtk::make_managed<Gtk::Label>("No recent files");
    recent_files_box->append(*label);
  } else {
    for (auto filepath : recent_files) {
      auto file = Gio::File::create_for_path(filepath);
      auto button = Gtk::make_managed<Gtk::Button>(file->get_basename());
      button->set_tooltip_text(filepath);

      recent_files_box->append(*button);
      button->signal_clicked().connect(
          sigc::bind(sigc::mem_fun(*state, &AppState::open_player), filepath));
    }
  }

  state->get_app()->add_window(*win);
  win->show();
}

//
void RecentsInstance::close() { win->close(); }
//
// /**
//  * @brief Callback for when a recent file is selected
//  *
//  * @param button The clicked button
//  * @param user_data The file path (gchar*)
//  */
// static void on_recent_file_clicked(GtkButton *button, gchar *filepath) {
//
//   g_return_if_fail(filepath != NULL);
//
//   g_print("opening recent file %s\n", filepath);
//
//   clean_new_on_playing(filepath);
// }
//
// /**
//  * @brief Opens the recent files dialog
//  *
//  * Creates and displays a window showing recently opened files (up to 5).
//  * Each file is shown as a clickable button that will open the file when
//  * clicked.
//  *
//  * @param action The action that triggered this (unused)
//  * @param parameter Action parameter (unused)
//  * @param app The application instance
//  */
// void open_recent_files(GSimpleAction *_action, GVariant *_parameter,
//                        AppData *app_data) {
//   g_return_if_fail(GTK_IS_APPLICATION(app_data->app));
//
//   // Load UI from resources
//   GtkBuilder *builder = load_builder(RECENTS_UI_PATH);
//   if (!builder) {
//     g_critical("Failed to load recent files UI");
//     return;
//   }
//
//   // Get window from builder
//   app_data->recents->win =
//       GTK_WINDOW(gtk_builder_get_object(builder, "recents"));
//   if (!app_data->recents->win) {
//     g_critical("Failed to get recent files window");
//     g_object_unref(builder);
//     return;
//   }
//
//   // Get container for recent files
//   GtkBox *recent_files_box =
//       GTK_BOX(gtk_builder_get_object(builder, "recent_files_box"));
//   if (!recent_files_box) {
//     g_critical("Failed to get recent files container");
//     g_object_unref(builder);
//     return;
//   }
//
//   // Get recent files from history
//   GList *recent_files = get_recent_files_list(MAX_RECENT_FILES);
//
//   if (!recent_files) {
//     // Add "No recent files" label if empty
//     GtkWidget *label = gtk_label_new("No recent files");
//     gtk_box_append(recent_files_box, label);
//   } else {
//     // Add buttons for each recent file
//     GList *iter = recent_files;
//     for (int i = 0; iter != NULL && i < MAX_RECENT_FILES;
//          i++, iter = iter->next) {
//       const gchar *filepath = (const gchar *)iter->data;
//
//       if (!g_file_query_exists(g_file_new_for_path(filepath), NULL)) {
//         continue;
//       }
//
//       GFile *file = g_file_new_for_path(filepath);
//
//       // Create button with filename (without path)
//       GtkWidget *button =
//       gtk_button_new_with_label(g_file_get_basename(file));
//       gtk_widget_set_tooltip_text(button, filepath);
//
//       // Connect click signal with full filepath as user_data
//       g_signal_connect(button, "clicked", G_CALLBACK(on_recent_file_clicked),
//                        strdup(filepath));
//
//       gtk_box_append(recent_files_box, button);
//       g_object_unref(file);
//     }
//
//     // Free the list (but not the strings - they're used as button data)
//     g_list_free(recent_files);
//   }
//
//   GtkButton *back_button =
//       GTK_BUTTON(gtk_builder_get_object(builder, "back_btn"));
//   if (!back_button) {
//     g_critical("Failed to get recent files window");
//     g_object_unref(builder);
//     return;
//   }
//   g_signal_connect(back_button, "clicked", G_CALLBACK(close_recents_window),
//                    app_data->recents->win);
//
//   // Show the window
//   gtk_application_add_window(GTK_APPLICATION(app_data->app),
//                              app_data->recents->win);
//   gtk_window_present(app_data->recents->win);
//
//   // Clean up builder
//   g_object_unref(builder);
// }
