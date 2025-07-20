#include <metadata.h>
#include <taglib/attachedpictureframe.h>
#include <taglib/fileref.h>
#include <taglib/id3v2tag.h>
#include <taglib/mpegfile.h>
#include <taglib/tag.h>

unsigned char *extract_album_art(const char *filepath, unsigned long *size) {
  TagLib::MPEG::File file(filepath);
  if (!file.isValid())
    return nullptr;

  TagLib::ID3v2::Tag *tag = file.ID3v2Tag();
  if (!tag)
    return nullptr;

  TagLib::ID3v2::FrameList frames = tag->frameList("APIC");
  if (frames.isEmpty())
    return nullptr;

  TagLib::ID3v2::AttachedPictureFrame *pic =
      dynamic_cast<TagLib::ID3v2::AttachedPictureFrame *>(frames.front());
  if (!pic)
    return nullptr;

  *size = pic->picture().size();
  unsigned char *data = (unsigned char *)malloc(*size);
  memcpy(data, pic->picture().data(), *size);
  return data;
}
