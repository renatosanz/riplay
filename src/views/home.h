#ifndef HOME
#define HOME
#include "types.h"

int show_home_view(AppData *app_data);
void open_new_file_dialog(GSimpleAction *action, GVariant *parameter,
                          AppData *app_data);

#endif // !HOME
