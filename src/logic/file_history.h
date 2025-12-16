#ifndef FILE_HISTORY_H
#define FILE_HISTORY_H

#include <glib.h>

#define MAX_HISTORY_ENTRIES 10
#define HISTORY_FILE "recent_files.ini"

void add_to_recent_files(const gchar *filepath);
GList *get_recent_files_list(guint max_items);
void clear_file_history();

#endif
