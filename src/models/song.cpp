#include "metadata/metadata.h"
#include "models.h"
#include "types.h"
#include <memory>

SongInstance::SongInstance(std::string filepath) {
  this->filepath = filepath;
  this->metadata = extract_metadata_from_path(filepath);
  auto extracted_lyrics_data = extractLyrics(this->filepath);

  if (std::holds_alternative<std::string>(extracted_lyrics_data)) {
    raw_lyrics = std::get<std::string>(extracted_lyrics_data);
    lyric_props = std::vector<LyricProp>({});
    sync_lyrics = parser_lyrics(raw_lyrics, lyric_props);
  }

  return;
}

std::string SongInstance::get_filepath() { return filepath; }
std::string SongInstance::get_raw_lyrics() { return raw_lyrics; }
std::vector<LyricBar> SongInstance::get_sync_lyrics() { return sync_lyrics; };
std::vector<LyricProp> SongInstance::get_lyrics_props() { return lyric_props; };
std::shared_ptr<FileMetadata> SongInstance::get_metadata() { return metadata; }
