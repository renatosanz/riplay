#ifndef PLAYER_H
#define PLAYER_H
#include "types.h"
#define MIN_IN_SECS 60

int clean_new_on_playing(const char *filename);
void free_global_resources(AppData *app_data);
int load_player_window(AppData *app_data);

#endif // !PLAYER_H
