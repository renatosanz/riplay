#include "models/models.h"

SongInstance::SongInstance(std::string filepath) {
  this->filepath = filepath;
  return;
}

std::string SongInstance::get_filepath() { return filepath; }
