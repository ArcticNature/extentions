// Copyright 2016 Stefano Pogliani <stefano@spogliani.net>
#include "ext/repository/git.h"

#include <git2.h>
#include <string>

#include "core/model/repository.h"
#include "ext/repository/git/exceptions.h"
#include "ext/repository/git/istream.h"

using sf::core::model::IStreamRef;
using sf::core::model::RepositoryVersionRef;

using sf::ext::exception::GitException;
using sf::ext::exception::GitInvalidVersion;
using sf::ext::exception::GitTypeError;

using sf::ext::repository::GitRepo;
using sf::ext::repository::GitRepoVersion;

using sf::ext::repository::GitBlobStream;
using sf::ext::repository::GitBlobStreamBuf;


void GitRepo::ensure_repo_is_open() {
  // Return if repository is open valid.
  if (this->repo) {
    return;
  }
  int error = git_repository_open(&this->repo, this->path.c_str());
  GitException::checkGitError(error);
}

git_commit* GitRepo::resolveCommit(std::string revision) {
  this->ensure_repo_is_open();

  // Get object named by revision.
  git_object* object = nullptr;
  int error = git_revparse_single(&object, this->repo, revision.c_str());
  GitException::checkGitError(error);

  // Check type is commit.
  if (git_object_type(object) != GIT_OBJ_COMMIT) {
    git_object_free(object);
    throw GitTypeError(GIT_OBJ_COMMIT, git_object_type(object));
  }
  return reinterpret_cast<git_commit*>(object);
}


GitRepo::GitRepo(std::string path, std::string latest) {
  this->latest_alias = latest;
  this->path = path;
  this->repo = nullptr;
}

GitRepo::~GitRepo() {
  if (this->repo) {
    git_repository_free(this->repo);
  }
}

RepositoryVersionRef GitRepo::latest() {
  return this->version(this->latest_alias);
}

RepositoryVersionRef GitRepo::version(std::string revision) {
  git_commit* commit = this->resolveCommit(revision);
  git_tree*   tree   = nullptr;

  int error = git_commit_tree(&tree, commit);
  git_commit_free(commit);

  GitException::checkGitError(error);
  return RepositoryVersionRef(new GitRepoVersion(this, tree));
}

std::string GitRepo::resolveVersion(const std::string version) {
  this->verifyVersion(version);
  git_commit* commit = nullptr;

  // Resolve <latest> special version.
  if (version == "<latest>") {
    commit = this->resolveCommit(this->latest_alias);
  } else {
    commit = this->resolveCommit(version);
  }

  // Get commit ID.
  char oidstr[GIT_OID_HEXSZ + 1];
  git_oid_tostr(oidstr, sizeof(oidstr), git_commit_id(commit));

  git_commit_free(commit);
  return std::string(oidstr);
}

void GitRepo::verifyVersion(const std::string version) {
  // Try to resolve the commit.
  try {
    // Resolve <latest> special version.
    if (version == "<latest>") {
      this->resolveCommit(this->latest_alias);
    } else {
      this->resolveCommit(version);
    }

  } catch (GitTypeError&) {
    throw GitInvalidVersion(version);

  } catch (GitException& exc) {
    // If the version cannot be parsed or the commit
    // cannot be found throw an exception.
    int code = exc.gitCode();
    if (code == GIT_ENOTFOUND || code == GIT_EAMBIGUOUS) {
      throw GitInvalidVersion(version);
    }

    // Otherwise propagate the original exception.
    throw;
  }
}


GitRepoVersion::GitRepoVersion(GitRepo* repo, git_tree* tree) {
  this->repo = repo;
  this->tree = tree;
}

GitRepoVersion::~GitRepoVersion() {
  git_tree_free(this->tree);
}

bool GitRepoVersion::exists(const std::string path) {
  git_tree_entry* entry = nullptr;
  int error = git_tree_entry_bypath(&entry, this->tree, path.c_str());
  if (error == GIT_ENOTFOUND) {
    return false;
  }

  GitException::checkGitError(error);
  git_tree_entry_free(entry);
  return true;
}

std::string GitRepoVersion::readFile(const std::string path) {
  // Get tree entry from path.
  git_tree_entry* entry = nullptr;
  int error = git_tree_entry_bypath(&entry, this->tree, path.c_str());
  GitException::checkGitError(error);

  // Check type is blob.
  if (git_tree_entry_type(entry) != GIT_OBJ_BLOB) {
    git_tree_entry_free(entry);
    throw GitTypeError(GIT_OBJ_BLOB, git_tree_entry_type(entry));
  }

  // Convert to blob.
  git_blob* blob = nullptr;
  error = git_blob_lookup(&blob, this->repo->repo, git_tree_entry_id(entry));
  git_tree_entry_free(entry);
  GitException::checkGitError(error);

  // Copy the blob as a string.
  const char* raw_string = reinterpret_cast<const char*>(
      git_blob_rawcontent(blob)
  );
  auto size = git_blob_rawsize(blob);
  return std::string(raw_string, size);
}

IStreamRef GitRepoVersion::streamFile(const std::string path) {
  // Get tree entry from path.
  git_tree_entry* entry = nullptr;
  int error = git_tree_entry_bypath(&entry, this->tree, path.c_str());
  GitException::checkGitError(error);

  // Check type is blob.
  if (git_tree_entry_type(entry) != GIT_OBJ_BLOB) {
    git_tree_entry_free(entry);
    throw GitTypeError(GIT_OBJ_BLOB, git_tree_entry_type(entry));
  }

  // Convert to blob.
  git_blob* blob = nullptr;
  error = git_blob_lookup(&blob, this->repo->repo, git_tree_entry_id(entry));
  git_tree_entry_free(entry);
  GitException::checkGitError(error);

  // Build stream and return.
  GitBlobStreamBuf* buffer = new GitBlobStreamBuf(blob);
  git_blob_free(blob);
  return IStreamRef(new GitBlobStream(buffer));
}
