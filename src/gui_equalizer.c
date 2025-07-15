#include "glib.h"
#include "gtk/gtk.h"
#include "gui.h"
#include "types.h"
#include <string.h>

#define EQUALIZER_UI_PATH "/org/riplay/data/ui/equalizer.ui"
#define EQUALIZER_BANDS 9
static GtkWindow *equalizer_win; // equalizer window
static gfloat frecs_range[EQUALIZER_BANDS] = {30,   64,   125,  250, 500,
                                              1000, 2000, 3000, 5000};

static gfloat band_marks[] = {0, 25, 50, 75, 100};
EqualizerSliderData *bands;
gboolean is_equalizer_enabled = false;

void toggle_enable_equalizer(GSimpleAction *action, GVariant *parameter,
                             GApplication *app) {
  is_equalizer_enabled = !is_equalizer_enabled;
  is_equalizer_enabled ? g_print("equalizer enabled!!!\n")
                       : g_print("equalizer disabled!!!\n");

  for (int i = 0; i < EQUALIZER_BANDS; i++) {
    gtk_widget_set_sensitive(GTK_WIDGET(bands[i].band_scale),
                             is_equalizer_enabled);
  }
}

void open_equalizer(GSimpleAction *action, GVariant *parameter,
                    GApplication *app) {
  g_return_if_fail(GTK_IS_APPLICATION(app));

  // load UI from resources
  GtkBuilder *builder = load_builder(EQUALIZER_UI_PATH);
  if (!builder) {
    g_critical("Failed to load recent files UI");
    return;
  }

  // Get window from builder
  equalizer_win = GTK_WINDOW(gtk_builder_get_object(builder, "equalizer_win"));
  if (!equalizer_win) {
    g_critical("Failed to get equalizer window");
    g_object_unref(builder);
    return;
  }

  // get the GtkBox that will contain all the frecuency bands
  GtkBox *bands_container =
      GTK_BOX(gtk_builder_get_object(builder, "bands_container"));
  if (!bands_container) {
    g_critical("Failed to get bands container ");
    g_object_unref(builder);
    return;
  }

  // for each frec create a band, includes a slider and label within a GtkBox
  bands = g_new0(EqualizerSliderData, EQUALIZER_BANDS);
  for (int i = 0; i < EQUALIZER_BANDS; i++) {
    bands[i].frecuency = frecs_range[i];

    GtkWidget *band_cont = GTK_WIDGET(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
    gtk_box_append(GTK_BOX(bands_container), band_cont);

    GtkWidget *band_scale = GTK_WIDGET(
        gtk_scale_new_with_range(GTK_ORIENTATION_VERTICAL, 0, 100, 1));
    gtk_widget_set_vexpand(band_scale, true);

    for (int i = 0; i < 5; i++) {
      gtk_scale_add_mark(GTK_SCALE(band_scale), band_marks[i], GTK_POS_TOP,
                         g_strdup_printf("<span>%0.f</span>", band_marks[i]));
    }

    GtkWidget *band_label = GTK_WIDGET(gtk_label_new(""));

    gtk_label_set_label(
        GTK_LABEL(band_label),
        bands[i].frecuency > 1000
            ? g_strdup_printf("%.0f KHz", (bands[i].frecuency / 1000))
            : g_strdup_printf("%.0f Hz", bands[i].frecuency));

    gtk_box_append(GTK_BOX(band_cont), band_label);
    gtk_box_append(GTK_BOX(band_cont), band_scale);

    bands[i].band_cont = GTK_BOX(band_cont);
    bands[i].band_scale = GTK_SCALE(band_scale);
    bands[i].band_label = GTK_LABEL(band_label);
  }

  // Show the window
  gtk_application_add_window(GTK_APPLICATION(app), equalizer_win);
  gtk_window_present(equalizer_win);

  // Clean up builder
  g_object_unref(builder);
}
