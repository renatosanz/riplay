#ifndef FILE_HISTORY_H
#define FILE_HISTORY_H

#include <glibmm.h>
#include <vector>

void add_to_recent_files(const gchar *filepath);
std::vector<Glib::ustring> get_recent_files_list(int max_items);

void clear_file_history();

#endif
