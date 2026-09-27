#include "models/models.h"
#include <metadata/metadata.h>
#include "adwaita.h"

int main(int argc, char *argv[]) {
  gst_init(&argc, &argv);
  adw_init();

  AppState *state = new AppState(argv, argc);

  return state->run(argc, argv);
}
