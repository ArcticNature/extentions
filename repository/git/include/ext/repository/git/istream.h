// Copyright 2016 Stefano Pogliani <stefano@spogliani.net>
#ifndef EXT_REPOSITORY_GIT_ISTREAM_H_
#define EXT_REPOSITORY_GIT_ISTREAM_H_

#include <git2.h>

#include <istream>
#include <streambuf>

#include "core/model/repository.h"


namespace sf {
namespace ext {
namespace repository {

  //! Readable stream buffer to wrap gitlib2 blobs.
  class GitBlobStreamBuf : public std::streambuf {
   protected:
    char* content;
    git_off_t size;

   public:
    explicit GitBlobStreamBuf(git_blob* blob);
    ~GitBlobStreamBuf();
  };

  //! Read stream to delete the underling GitBlobStreamBuf.
  class GitBlobStream : public std::istream {
   protected:
    GitBlobStreamBuf* buffer;

   public:
    explicit GitBlobStream(GitBlobStreamBuf* buffer);
    ~GitBlobStream();
  };

}  // namespace repository
}  // namespace ext
}  // namespace sf

#endif  // EXT_REPOSITORY_GIT_ISTREAM_H_
