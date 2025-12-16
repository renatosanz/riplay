/**
 * @file memory.c
 * @brief Global resource management and cleanup
 *
 * This module handles the proper cleanup of all application resources
 * and memory management for the AppData structure.
 */

#include "types.h"
#include <glib-object.h>

/**
 * @brief Frees all resources associated with the AppData structure
 *
 * Safely releases all memory and resources owned by the AppData structure,
 * including:
 * - Media stream objects
 * - Metadata and its properties
 * - Filename strings
 * - The AppData structure itself
 *
 * @param app_data Pointer to the AppData structure to clean up
 *
 * @note This function is NULL-safe and will handle partially initialized
 * structures
 * @note After calling this function, the app_data pointer should be set to NULL
 *       by the caller if it won't be used anymore
 */
void free_global_resources(AppData *app_data) {
  // Early return if NULL pointer passed
  if (!app_data) {
    return;
  }

  // Release media stream if it exists
  if (app_data->media_stream && G_IS_OBJECT(app_data->media_stream)) {
    g_object_unref(app_data->media_stream);
    app_data->media_stream = NULL;
  }

  // Clean up metadata structure
  if (app_data->metadata) {
    // Free metadata properties if they exist
    if (app_data->metadata->properties) {
      g_free(app_data->metadata->properties);
      app_data->metadata->properties = NULL;
    }

    // Free metadata structure itself
    g_free(app_data->metadata);
    app_data->metadata = NULL;
  }

  // Free filename string if it exists
  if (app_data->filename) {
    g_free(app_data->filename);
    app_data->filename = NULL;
  }

  // Free the main app data structure
  g_free(app_data);

  // Note: The caller should set their pointer to NULL after this call
  // as we can't modify their pointer value here
}
