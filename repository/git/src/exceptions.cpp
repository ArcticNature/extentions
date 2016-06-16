// Copyright 2015 Stefano Pogliani <stefano@spogliani.net>
#include <git2.h>
#include <string>

#include "ext/repository/git.h"
#include "ext/repository/git/exceptions.h"

using sf::core::exception::SfException;

using sf::ext::exception::GitException;
using sf::ext::exception::GitInvalidVersion;
using sf::ext::exception::GitTypeError;


GitException::GitException(
    int code, int klass, std::string message
) : SfException(
    "libgit error: " + message
) {
  this->libgit_code    = code;
  this->libgit_class   = klass;
  this->libgit_message = message;
}

int GitException::getCode() const {
  return -4100 + this->libgit_code;
}

int GitException::gitClass() const {
  return this->libgit_class;
}

int GitException::gitCode() const {
  return this->libgit_code;
}

std::string GitException::gitMessage() const {
  return this->libgit_message;
}


GitInvalidVersion::GitInvalidVersion(std::string version) : SfException(
  "Invalid version '" + version + "'"
) { }

int GitInvalidVersion::getCode() const {
  return -4001;
}


GitTypeError::GitTypeError(git_otype expected, git_otype actual) : SfException(
  "Invalid libgit2 object type: " +
  std::string(git_object_type2string(expected)) + " expected but " +
  std::string(git_object_type2string(actual)) + " found"
) { }

int GitTypeError::getCode() const {
  return -4000;
}


void GitException::checkGitError(int code) {
  if (code == 0) {
    return;
  }
  const git_error* err = giterr_last();
  throw GitException(code, err->klass, err->message);
}
