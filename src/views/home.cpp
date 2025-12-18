#include "cairomm/context.h"
#include "gdkmm/rgba.h"
#include "glibmm/main.h"
#include "glibmm/refptr.h"
#include "gtkmm/drawingarea.h"
#include "gtkmm/window.h"
#include "models/models.h"
#include "sigc++/functors/mem_fun.h"
#include "utils.h"
#include <memory>
#include <sched.h>

HomeInstance::HomeInstance(AppState *state) {
  printf("Creating HomeInstance()...\n");
  this->state = state;

  auto builder = load_builder("/org/riplay/data/ui/home.ui");
  win = builder->get_object<Gtk::Window>("default_window");

  // set up standby animation
  drawing_area = builder->get_object<Gtk::DrawingArea>("spectrum_default");
  drawing_area->set_draw_func(
      sigc::mem_fun(*this, &HomeInstance::draw_stand_by_function));

  // Start animation timer (32ms interval ≈ 30fps)
  timeout_id = Glib::signal_timeout().connect(
      sigc::mem_fun(*this, &HomeInstance::on_timeout), 500);
  state->add_window(*win);
}

void HomeInstance::show() { win->show(); }

void HomeInstance::close() {
  if (win) {
    win->close();
  }
}

HomeInstance::~HomeInstance() {
  if (timeout_id.connected()) {
    timeout_id.disconnect();
  }
}

bool HomeInstance::on_timeout() {

  // update animation position (resets after 500 frames)
  this->position = (this->position + 1) % 500;

  // trigger redraw
  this->drawing_area->queue_draw();

  return TRUE;
}

void HomeInstance::draw_stand_by_function(
    const std::shared_ptr<Cairo::Context> &cr, int width, int height) {
  Gdk::RGBA color;

  const float frequency = 1.0f / 20.0f;   // Wave frequency
  const float mid_height = height / 2.0f; // Vertical center

  // set drawing properties
  cr->set_source_rgb(1, 1, 1);
  cr->set_line_width(5);

  // draw sine wave
  for (int i = 1; i <= width; i += 3) {
    float y_offset = -20 * sin((position + i) * frequency);
    cr->line_to(i, mid_height + y_offset);
  }
  cr->stroke();

  // fill with widget's background color
  color = drawing_area->get_color();
  cr->set_source_rgba(color.get_red(), color.get_green(), color.get_blue(),
                      color.get_alpha());
  cr->fill();
}

// void file_dialog_response(GObject *source_object, GAsyncResult *result,
//                           AppData *app_data) {
//   GtkFileDialog *dialog = GTK_FILE_DIALOG(source_object);
//   GError *error = NULL;
//
//   // Get selected file
//   GFile *file = gtk_file_dialog_open_finish(dialog, result, &error);
//
//   if (error) {
//     g_printerr("File selection error: %s\n", error->message);
//     g_error_free(error);
//     return;
//   }
//
//   if (!file) {
//     g_print("No file selected\n");
//     return;
//   }
//
//   // Get file path
//   char *filepath = g_file_get_path(file);
//   if (!filepath) {
//     g_object_unref(file);
//     g_critical("Failed to get file path");
//     return;
//   }
//
//   // add filepath to file history
//   add_to_recent_files(filepath);
//
//   // Start new playback session
//   clean_new_on_playing(filepath);
//
//   // Clean up
//   g_free(filepath);
//   g_object_unref(file);
// }
//
// /**
//  * @brief Opens file selection dialog
//  *
//  * Creates and displays a file dialog for selecting audio files.
//  *
//  * @param action The action that triggered this (unused)
//  * @param parameter Action parameter (unused)
//  * @param app The application instance
//  */
// void open_new_file_dialog(GSimpleAction *action, GVariant *parameter,
//                           AppData *app_data) {
//   (void)action;
//   (void)parameter;
//
//   GtkWindow *window = GTK_WINDOW(app_data->home->win);
//   GtkFileDialog *dialog = gtk_file_dialog_new();
//
//   // Configure dialog
//   gtk_file_dialog_set_title(dialog, "Select a song");
//
//   // Set audio file filter
//   GtkFileFilter *filter = gtk_file_filter_new();
//   gtk_file_filter_add_mime_type(filter, "audio/*");
//   gtk_file_dialog_set_default_filter(dialog, filter);
//
//   // Show dialog
//   gtk_file_dialog_open(dialog, window, NULL,
//                        GAsyncReadyCallback(file_dialog_response), app_data);
// }
