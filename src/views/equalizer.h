#ifndef EQUALIZER_H
#define EQUALIZER_H
#include "gio/gio.h"
#include "types.h"
void open_equalizer(GSimpleAction *action, GVariant *parameter,
                    AppData *app_data);
void toggle_enable_equalizer(GSimpleAction *action, GVariant *parameter,
                             AppData *app_data);
void close_equalizer(GSimpleAction *action, GVariant *parameter,
                     AppData *app_data);
#endif // !EQUALIZER_H
