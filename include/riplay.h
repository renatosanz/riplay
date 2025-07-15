#ifndef RIPLAY_H
#define RIPLAY_H
#include "types.h"

int clean_new_on_playing(const char *filename);
void free_global_resources(AppData *app_data);

#endif // !RIPLAY_H
