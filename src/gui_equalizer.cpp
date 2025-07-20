#include "glib.h"
#include "gtk/gtk.h"
#include "gui.h"
#include "types.h"
#include <stdio.h>
#include <string.h>

#define EQUALIZER_UI_PATH "/org/riplay/data/ui/equalizer.ui"
#define EQUALIZER_BANDS 9
static GtkWindow *equalizer_win; // equalizer window
static gfloat frecs_range[EQUALIZER_BANDS] = {30,   64,   125,  250, 500,
                                              1000, 2000, 3000, 5000};

static EqualizerSliderData *bands = NULL;
gboolean is_equalizer_enabled = false;

void toggle_enable_equalizer(GSimpleAction *action, GVariant *parameter,
                             GApplication *app) {
  is_equalizer_enabled = !is_equalizer_enabled;
  for (int i = 0; i < EQUALIZER_BANDS; i++) {
    gtk_widget_set_sensitive(GTK_WIDGET(bands[i].band_scale),
                             is_equalizer_enabled);
  }
}

static void on_scale_value_changed(GtkScale *scale, EqualizerSliderData *data) {
  data->value = gtk_range_get_value(GTK_RANGE(scale));
}

void close_equalizer(GSimpleAction *action, GVariant *parameter,
                     GApplication *app) {
  gtk_window_destroy(GTK_WINDOW(equalizer_win));
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

  GtkSwitch *on_off_toggle =
      GTK_SWITCH(gtk_builder_get_object(builder, "on_off_toggle"));
  if (!on_off_toggle) {
    g_critical("Failed to get bands container ");
    g_object_unref(builder);
    return;
  }

  gtk_switch_set_active(on_off_toggle, is_equalizer_enabled);

  // for each frec create a band, includes a slider and label within a GtkBox
  if (!bands) {
    bands = g_new0(EqualizerSliderData, EQUALIZER_BANDS);
    for (int i = 0; i < EQUALIZER_BANDS; i++) {
      bands[i].default_value = 50;
      bands[i].min_value = 0;
      bands[i].max_value = 100;
      bands[i].value = bands[i].default_value;
    }
  }
  for (int i = 0; i < EQUALIZER_BANDS; i++) {
    bands[i].frecuency = frecs_range[i];

    GtkWidget *band_cont = GTK_WIDGET(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
    gtk_box_append(GTK_BOX(bands_container), band_cont);

    GtkWidget *band_scale = GTK_WIDGET(gtk_scale_new_with_range(
        GTK_ORIENTATION_VERTICAL, bands[i].min_value, bands[i].max_value, 1));
    gtk_widget_set_vexpand(band_scale, true);
    gtk_range_set_value(GTK_RANGE(band_scale), bands[i].value);
    gtk_scale_set_draw_value(GTK_SCALE(band_scale), true);
    gtk_widget_set_sensitive(GTK_WIDGET(band_scale), is_equalizer_enabled);
    g_signal_connect(GTK_WIDGET(band_scale), "value-changed",
                     G_CALLBACK(on_scale_value_changed), &bands[i]);

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
