// Copyright 2015 Stefano Pogliani <stefano@spogliani.net>
#include <algorithm>

#include "ext/repository/git/istream.h"

using sf::ext::repository::GitBlobStreamBuf;
using sf::ext::repository::GitBlobStream;


GitBlobStreamBuf::GitBlobStreamBuf(git_blob* blob) {
  const char* src = reinterpret_cast<const char*>(git_blob_rawcontent(blob));
  this->size = git_blob_rawsize(blob);
  this->content = nullptr;

  if (this->size != 0) {
    this->content = new char[this->size];
    std::copy(src, src + this->size, this->content);
  }

  this->setg(this->content, this->content, this->content + this->size);
}

GitBlobStreamBuf::~GitBlobStreamBuf() {
  if (this->content) {
    delete [] content;
  }
}


GitBlobStream::GitBlobStream(GitBlobStreamBuf* buffer) : std::istream(buffer) {
  this->buffer = buffer;
}

GitBlobStream::~GitBlobStream() {
  delete this->buffer;
}
