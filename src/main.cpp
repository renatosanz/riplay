#include "models/models.h"
#include <metadata/metadata.h>

int main(int argc, char *argv[]) {
  AppState *state = new AppState(argv, argc);

  return state->run(argc, argv);
}
