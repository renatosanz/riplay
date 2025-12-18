#include "metadata/metadata.h"
#include "models.h"
#include "types.h"
#include <memory>

SongInstance::SongInstance(std::string filepath) {
  this->filepath = filepath;
  this->metadata = extract_metadata_from_path(filepath);
  return;
}

std::string SongInstance::get_filepath() { return filepath; }

std::shared_ptr<FileMetadata> SongInstance::get_metadata() { return metadata; }
