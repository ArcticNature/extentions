// Copyright 2015 Stefano Pogliani <stefano@spogliani.net>
#include <gtest/gtest.h>
#include <git2.h>

#include "ext/repository/git/exceptions.h"

using sf::ext::exception::GitException;


TEST(GitRepo, ErrorCheck) {
  git_libgit2_init();
  git_repository* repo = nullptr;
  int code = git_repository_open(&repo, "not a real repo");

  ASSERT_NE(0, code);
  ASSERT_THROW(GitException::checkGitError(code), GitException);

  try {
    GitException::checkGitError(code);

  } catch (GitException& ex) {
    ASSERT_EQ(-4103, ex.getCode());
    ASSERT_EQ(-3, ex.gitCode());
    ASSERT_EQ(
        "Failed to resolve path 'not a real repo': No such file or directory",
        ex.gitMessage()
    );
  }
}
