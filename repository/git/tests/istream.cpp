// Copyright 2016 Stefano Pogliani <stefano@spogliani.net>
#include <gtest/gtest.h>

#include "ext/repository/git/exceptions.h"
#include "ext/repository/git/istream.h"

using sf::ext::exception::GitException;
using sf::ext::exception::GitTypeError;

using sf::ext::repository::GitBlobStreamBuf;
using sf::ext::repository::GitBlobStream;

const std::string BRANCH_NAME = "ext.repo.git.fixture";


class GitBlobStreamBufTest : public ::testing::Test {
 protected:
  git_repository* repo;

 public:
  GitBlobStreamBufTest() {
    git_libgit2_init();
    this->repo = nullptr;
  }

  ~GitBlobStreamBufTest() {
    if (this->repo) {
      git_repository_free(this->repo);
    }
  }

  git_repository* getRepo() {
    if (this->repo == nullptr) {
      int error = git_repository_open(&this->repo, "../snow-fox");
      GitException::checkGitError(error);
    }
    return this->repo;
  }

  git_blob* getBlob(std::string path, std::string revision=BRANCH_NAME) {
    // Ensure repo is loaded.
    this->getRepo();

    // Get object named by revision.
    git_object* object = nullptr;
    int error = git_revparse_single(&object, this->repo, revision.c_str());
    GitException::checkGitError(error);

    // Check type is commit.
    if (git_object_type(object) != GIT_OBJ_COMMIT) {
      git_object_free(object);
      throw GitTypeError(GIT_OBJ_COMMIT, git_object_type(object));
    }

    // Get tree from commit.
    git_commit* commit = (git_commit*)object;
    git_tree*   tree   = nullptr;
    error = git_commit_tree(&tree, commit);
    git_commit_free(commit);
    GitException::checkGitError(error);

    // Get tree entry from path.
    git_tree_entry* entry = nullptr;
    error = git_tree_entry_bypath(&entry, tree, path.c_str());
    git_tree_free(tree);
    GitException::checkGitError(error);

    // Check type is blob.
    if (git_tree_entry_type(entry) != GIT_OBJ_BLOB) {
      git_tree_entry_free(entry);
      throw GitTypeError(GIT_OBJ_BLOB, git_tree_entry_type(entry));
    }

    // Convert to blob.
    git_blob* blob = nullptr;
    error = git_blob_lookup(&blob, this->repo, git_tree_entry_id(entry));
    git_tree_entry_free(entry);
    GitException::checkGitError(error);

    return blob;
  }
};


TEST_F(GitBlobStreamBufTest, ReadEmptyBlob) {
  git_blob* blob = this->getBlob("fixtures/exists");
  GitBlobStreamBuf buffer(blob);
  git_blob_free(blob);
  
  std::istream stream(&buffer);
  ASSERT_EQ(-1, stream.get());
  ASSERT_TRUE(stream.eof());
}

TEST_F(GitBlobStreamBufTest, ReadBlob) {
  git_blob* blob = this->getBlob("fixtures/content");
  GitBlobStreamBuf buffer(blob);
  git_blob_free(blob);

  std::istream stream(&buffer);
  std::string text;
  std::getline(stream, text);
  ASSERT_EQ("File with some content", text);
}

TEST_F(GitBlobStreamBufTest, ReadAllBlob) {
  git_blob* blob = this->getBlob("fixtures/content");
  GitBlobStreamBuf buffer(blob);
  git_blob_free(blob);

  std::istream stream(&buffer);
  std::string text;
  int num;

  std::getline(stream, text);
  ASSERT_EQ("File with some content", text);
  std::getline(stream, text);
  ASSERT_EQ("And a couple of lines", text);
  
  stream >> num;
  ASSERT_EQ(12345, num);
  
  stream >> text;
  ASSERT_EQ("true", text);
  stream >> text;
  ASSERT_EQ("false", text);
}

TEST_F(GitBlobStreamBufTest, ReadBlobThroughStream) {
  git_blob* blob = this->getBlob("fixtures/content");
  GitBlobStreamBuf* buffer = new GitBlobStreamBuf(blob);
  git_blob_free(blob);

  GitBlobStream stream(buffer);
  std::string text;
  std::getline(stream, text);
  ASSERT_EQ("File with some content", text);
}
