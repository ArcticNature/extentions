// Copyright 2016 Stefano Pogliani <stefano@spogliani.net>
#include <gtest/gtest.h>

#include <memory>

#include "core/model/repository.h"
#include "ext/repository/git.h"
#include "ext/repository/git/exceptions.h"

//#include "testing/options.h"

//using sf::exception::ErrNoException;
using sf::core::model::IStreamRef;

using sf::ext::exception::GitException;
using sf::ext::exception::GitInvalidVersion;
using sf::ext::repository::GitRepo;

typedef std::shared_ptr<GitRepo> GitRepoRef;


const std::string NOT_COMMIT = "0000000393468508e310fef2af3ea38ae6e37db5";

const std::string BRANCH_NAME   = "ext.repo.git.fixture";
const std::string BRANCH_TREE   = "4c9ef4528bb8775a0d5a2c9d0ec838279aa2066b";
const std::string BRANCH_COMMIT = "1e57e7732f46f526ae1166f4653a83964e75e3a6";
const std::string TAG_NAME      = "v0.0.3";
const std::string TAG_COMMIT    = "9bf19d1c8f978927b8c85bd4ec0b8ed5426da4b8";


class GitRepoTest : public ::testing::Test {
 protected:
  std::string cwd;

 public:
  GitRepoTest() {
    git_libgit2_init();
    char* cwd = get_current_dir_name();
    this->cwd = std::string(cwd);
    free(cwd);
  }

  GitRepoRef make(std::string path="<cwd>") {
    if (path == "<cwd>") {
      path = this->cwd;
    }
    return GitRepoRef(new GitRepo(path));
  }

  GitRepoRef getFixture() {
    return this->make("../snow-fox");
  }
};


TEST_F(GitRepoTest, RepoNotFound) {
  GitRepoRef repo = this->make("not a valid repo");
  ASSERT_THROW(repo->latest(), GitException);
}

TEST_F(GitRepoTest, PathNotFound) {
  GitRepoRef repo = this->getFixture();
  ASSERT_FALSE(repo->latest()->exists("path does not exists"));
}

TEST_F(GitRepoTest, PathExists) {
  GitRepoRef repo = this->getFixture();
  ASSERT_TRUE(repo->latest()->exists("Readme.md"));
}

TEST_F(GitRepoTest, ResolveCommitToItself) {
  GitRepoRef  repo = this->getFixture();
  std::string branch = repo->resolveVersion(BRANCH_COMMIT);
  ASSERT_EQ(BRANCH_COMMIT, branch);
}

TEST_F(GitRepoTest, ResolveBranchName) {
  GitRepoRef  repo = this->getFixture();
  std::string branch = repo->resolveVersion(BRANCH_NAME);
  ASSERT_EQ(BRANCH_COMMIT, branch);
}

TEST_F(GitRepoTest, ResolveBranchLong) {
  GitRepoRef  repo = this->getFixture();
  std::string branch = repo->resolveVersion("refs/heads/" + BRANCH_NAME);
  ASSERT_EQ(BRANCH_COMMIT, branch);
}

TEST_F(GitRepoTest, ResolveTag) {
  GitRepoRef  repo = this->getFixture();
  std::string tag  = repo->resolveVersion(TAG_NAME);
  ASSERT_EQ(TAG_COMMIT, tag);
}

TEST_F(GitRepoTest, VerifyCommitNotFound) {
  GitRepoRef repo = this->getFixture();
  ASSERT_THROW(repo->verifyVersion(NOT_COMMIT), GitInvalidVersion);
}

TEST_F(GitRepoTest, VerifyCommitValid) {
  GitRepoRef repo = this->getFixture();
  repo->verifyVersion(BRANCH_COMMIT);
}

TEST_F(GitRepoTest, VerifyFailWithTree) {
  GitRepoRef repo = this->getFixture();
  ASSERT_THROW(repo->verifyVersion(BRANCH_TREE), GitInvalidVersion);
}

TEST_F(GitRepoTest, StreamFileForTag) {
  GitRepoRef repo = this->getFixture();
  IStreamRef stream = repo->version(BRANCH_NAME)->streamFile(
      "fixtures/exists"
  );
  ASSERT_NE(nullptr, stream);

  int ch = stream->get();
  EXPECT_EQ(-1, ch);
  EXPECT_TRUE(stream->eof());
}

TEST_F(GitRepoTest, StreamFileRaw) {
  GitRepoRef repo = this->getFixture();
  IStreamRef stream = repo->version(BRANCH_NAME)->streamFile(
      "fixtures/content"
  );
  ASSERT_NE(nullptr, stream);

  std::string line;
  std::getline(*stream, line);
  EXPECT_EQ("File with some content", line);
}
