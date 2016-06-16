// Copyright 2015 Stefano Pogliani <stefano@spogliani.net>
#include <git2.h>

#include "core/context/context.h"
#include "core/context/static.h"
#include "core/interface/lifecycle.h"

#include "core/model/cli-parser.h"
#include "core/model/logger.h"
#include "core/model/repository.h"

#include "core/registry/repositories.h"
#include "ext/repository/git.h"

using sf::core::context::Context;
using sf::core::context::Static;

using sf::core::interface::BaseLifecycleArg;
using sf::core::interface::BaseLifecycleHandler;

using sf::core::model::CLIParser;
using sf::core::model::LogInfo;
using sf::core::model::RepositoryRef;

using sf::core::registry::ReposRegistry;
using sf::ext::repository::GitRepo;


RepositoryRef git_repo_factotry() {
  CLIParser*  parser = Static::parser();
  std::string path = parser->getString("repo-path");

  LogInfo info = {{"path", path}};
  INFOV(
      Context::logger(),
      "Using GIT configuration repository from '${path}'", info
  );

  return RepositoryRef(new GitRepo(path));
}


class GitModuleInit : public BaseLifecycleHandler {
 public:
  void handle(std::string event, BaseLifecycleArg* arg) {
    git_libgit2_init();
    ReposRegistry::RegisterFactory("git", git_repo_factotry);
  }
};


class GitModuleExit : public BaseLifecycleHandler {
 public:
  void handle(std::string event, BaseLifecycleArg* arg) {
    git_libgit2_shutdown();
  }
};


LifecycleStaticOn("process::init", GitModuleInit);
LifecycleStaticOn("process::exit", GitModuleExit);
