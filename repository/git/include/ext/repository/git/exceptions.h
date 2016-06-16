// Copyright 2016 Stefano Pogliani <stefano@spogliani.net>
#ifndef EXT_REPOSITORY_GIT_EXCEPTIONS_H_
#define EXT_REPOSITORY_GIT_EXCEPTIONS_H_

#include <git2.h>
#include <string>

#include "core/exceptions/base.h"


namespace sf {
namespace ext {
namespace exception {

  //! Wrapper exception for libgit errors.
  class GitException : public sf::core::exception::SfException {
   protected:
    int libgit_class;
    int libgit_code;
    std::string libgit_message;

   public:
    GitException(int code, int klass, std::string message);
    int getCode() const;

    //! Returns the libgit error class.
    int gitClass() const;

    //! Returns the libgit error code.
    int gitCode() const;

    //! Returns the libgit error message.
    std::string gitMessage() const;

   public:
    //! Checks for errors in the last libgit call.
    /*!
     * If an error code is returned this function converts it
     * into an exception with the appropriate code and message.
     */
    static void checkGitError(int code);
  };

  //! Thrown when version verification fails.
  MSG_EXCEPTION(sf::core::exception::SfException, GitInvalidVersion);

  //! Thrown when an id resolves to the wrong libgit2 type.
  class GitTypeError : public sf::core::exception::SfException {
   public:
    GitTypeError(git_otype expected, git_otype actual);
    int getCode() const;
  };

}  // namespace exception
}  // namespace ext
}  // namespace sf

#endif  // EXT_REPOSITORY_GIT_EXCEPTIONS_H_
