// Copyright 2016 Stefano Pogliani <stefano@spogliani.net>
#ifndef EXT_REPOSITORY_GIT_H_
#define EXT_REPOSITORY_GIT_H_

#include <git2.h>
#include <string>

#include "core/model/repository.h"


namespace sf {
namespace ext {
namespace repository {

  class GitRepoVersion;

  //! Git backed repository.
  /*!
   * An instance of this class can be backed by a bare git
   * repository or a working directory.
   * 
   * A custom alias to resolve the <latest> version can be
   * specified, the default is master.
   */
  class GitRepo : public sf::core::model::Repository {
    friend class GitRepoVersion;

   protected:
    std::string latest_alias;
    std::string path;
    git_repository* repo;

    //! Ensures that the repository is initialised.
    void ensure_repo_is_open();

    //! Resolves the given revision to a commit.
    git_commit* resolveCommit(std::string revision);

   public:
    explicit GitRepo(
        std::string path, std::string latest = "refs/heads/master"
    );
    ~GitRepo();

    virtual sf::core::model::RepositoryVersionRef latest();
    virtual sf::core::model::RepositoryVersionRef version(
        std::string revision
    );

    virtual std::string resolveVersion(const std::string version);
    virtual void verifyVersion(const std::string version);
  };

  //! GitRepo version manager.
  class GitRepoVersion : public sf::core::model::RepositoryVersion {
   protected:
    GitRepo*  repo;
    git_tree* tree;

   public:
    GitRepoVersion(GitRepo* repo, git_tree* tree);
    ~GitRepoVersion();

    virtual bool exists(const std::string path);
    virtual std::string readFile(const std::string path);
    virtual sf::core::model::IStreamRef streamFile(const std::string path);
  };

}  // namespace repository
}  // namespace ext
}  // namespace sf

#endif  // EXT_REPOSITORY_GIT_H_
