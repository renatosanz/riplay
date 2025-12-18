#include "file_history.h"
#include "glibmm/fileutils.h"
#include <algorithm>
#include <glib/gstdio.h>
#include <glibmm.h>
#include <vector>

#define MAX_HISTORY_ENTRIES 10
#define HISTORY_FILE "recent_files.ini"
// returns the path of the config file, in linux equals
// ~/.config/riplay/recent_files.ini
std::string get_history_path() {
  return Glib::build_filename(Glib::get_user_config_dir(), "riplay",
                              HISTORY_FILE);
}

// recieve a filepath and adds it to history file
void add_to_recent_files(const gchar *filepath) {
  // GKeyFile *keyfile = g_key_file_new();
  // gchar *path = get_history_path();
  //
  // // create directory if not exists
  // g_mkdir_with_parents(g_get_user_config_dir(), 0755);
  //
  // // load existing file
  // if (g_file_test(path, G_FILE_TEST_EXISTS)) {
  //   g_key_file_load_from_file(keyfile, path, G_KEY_FILE_NONE, NULL);
  // }
  //
  // // get current list
  // gsize length = 0;
  // gchar **existing =
  //     g_key_file_get_string_list(keyfile, "History", "Files", &length, NULL);
  //
  // // check if file already exists in history, if is in the list then return
  // and
  // // free memory
  // for (gsize i = 0; i < length; i++) {
  //   if (g_strcmp0(existing[i], filepath) == 0) {
  //     g_strfreev(existing);
  //     g_key_file_free(keyfile);
  //     g_free(path);
  //     return;
  //   }
  // }
  //
  // // prepend new file and place it first on the list
  // gchar **new_list = g_new0(gchar *, length + 2);
  // new_list[0] = g_strdup(filepath);
  // for (gsize i = 0; i < length; i++) {
  //   new_list[i + 1] = existing[i];
  // }
  //
  // // remove the extra results, more than MAX_HISTORY_ENTRIES
  // if (length >= MAX_HISTORY_ENTRIES) {
  //   g_free(new_list[MAX_HISTORY_ENTRIES]);
  //   new_list[MAX_HISTORY_ENTRIES] = NULL;
  // }
  //
  // // save after save the new file
  // g_key_file_set_string_list(keyfile, "History", "Files",
  //                            (const gchar *const *)new_list,
  //                            MIN(length + 1, MAX_HISTORY_ENTRIES));
  // g_key_file_save_to_file(keyfile, path, NULL);
  //
  // // cleanup
  // g_strfreev(new_list);
  // g_key_file_free(keyfile);
  // g_free(path);
}

std::vector<Glib::ustring> get_recent_files_list(int max_items) {
  auto keyfile = Glib::KeyFile::create();
  std::string path = get_history_path();
  auto list = std::vector<Glib::ustring>();

  // check if file exists
  if (Glib::file_test(path, Glib::FileTest::EXISTS)) {
    // take all the saved paths and return them as GList
    keyfile->load_from_file(path);

    auto filenames = keyfile->get_string_list("History", "Files");

    for (auto filename : filenames) {
      // verify songs exists
      if (!Glib::file_test(filename, Glib::FileTest::EXISTS)) {
        continue;
      }
      list.push_back(filename);
    }
  }

  // cleanup
  std::reverse(list.begin(), list.end());
  return list; // return oldest first
}

// clear all history data from files
void clear_file_history() {
  // gchar *path = get_history_path();
  // g_unlink(path);
  // g_free(path);
}
