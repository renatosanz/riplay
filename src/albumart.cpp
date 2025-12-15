#include "glib/gprintf.h"
#include "glibconfig.h"
#include "tstringlist.h"
#include "types.h"
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <memory>
#include <metadata.h>
#include <ostream>
#include <regex>
#include <stdexcept> // Para manejo de errores
#include <string>
#include <taglib/attachedpictureframe.h>
#include <taglib/fileref.h>
#include <taglib/flacfile.h>
#include <taglib/id3v2frame.h>
#include <taglib/id3v2header.h>
#include <taglib/id3v2tag.h>
#include <taglib/mp4file.h>
#include <taglib/mpegfile.h>
#include <taglib/tag.h>
#include <taglib/tpropertymap.h>
#include <taglib/unsynchronizedlyricsframe.h>
#include <variant>
#include <vector>
#include <xiphcomment.h>

unsigned char *extractAlbumArt(const char *filePath, unsigned long *size) {
  TagLib::FileRef file(filePath);

  if (!file.isNull() && file.tag()) {
    // handle MP3 files (ID3v2 tags)
    if (TagLib::MPEG::File *mpegFile =
            dynamic_cast<TagLib::MPEG::File *>(file.file())) {
      if (mpegFile->ID3v2Tag()) {
        TagLib::ID3v2::FrameList frames =
            mpegFile->ID3v2Tag()->frameList("APIC");
        if (!frames.isEmpty()) {
          TagLib::ID3v2::AttachedPictureFrame *picFrame =
              static_cast<TagLib::ID3v2::AttachedPictureFrame *>(
                  frames.front());
          *size = picFrame->picture().size();
          unsigned char *data = (unsigned char *)malloc(*size);
          memcpy(data, picFrame->picture().data(), *size);
          return data;
        }
      }
    }
    // handle FLAC files
    else if (TagLib::FLAC::File *flacFile =
                 dynamic_cast<TagLib::FLAC::File *>(file.file())) {
      const TagLib::List<TagLib::FLAC::Picture *> &pictures =
          flacFile->pictureList();
      if (!pictures.isEmpty()) {
        *size = pictures[0]->data().size();
        unsigned char *data = (unsigned char *)malloc(*size);
        memcpy(data, pictures[0]->data().data(), *size);
        return data;
      }
    }
    // handle aac/mp4 files
    else if (TagLib::MP4::File *mp4File =
                 dynamic_cast<TagLib::MP4::File *>(file.file())) {
      if (mp4File->tag()) {
        TagLib::MP4::Tag *tag = mp4File->tag();
        if (tag->contains("covr")) {
          TagLib::MP4::Item coverItem = tag->item("covr");
          TagLib::MP4::CoverArtList coverArtList = coverItem.toCoverArtList();
          if (!coverArtList.isEmpty()) {
            *size = coverArtList[0].data().size();
            unsigned char *data = (unsigned char *)malloc(*size);
            memcpy(data, coverArtList[0].data().data(), *size);
            return data;
          }
        }
      }
    }
  }
  return 0;
}
