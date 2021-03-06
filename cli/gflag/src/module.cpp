// Copyright 2015 Stefano Pogliani <stefano@spogliani.net>
#include <string>

#include "core/interface/lifecycle.h"
#include "core/lifecycle/process.h"
#include "core/registry/cli-parser.h"
#include "ext/cli/gflags.h"

using sf::core::interface::BaseLifecycleArg;
using sf::core::interface::BaseLifecycleHandler;
using sf::core::interface::Lifecycle;
using sf::core::interface::LifecycleHandlerRef;

using sf::core::lifecycle::Process;

using sf::core::model::CLIParser;
using sf::core::registry::CLIParsers;


sf::core::model::CLIParser* gflags_factory() {
  return new sf::ext::cli::GFlagsCLI();
}

//! Module initialiser for the GFlags module.
class CliGFlagsInitHandler : public BaseLifecycleHandler {
 public:
  void handle(std::string event, BaseLifecycleArg*) {
    CLIParsers::RegisterFactory("gflags", gflags_factory);
  }
};


// Register the initialiser.
LifecycleStaticOn("process::init", CliGFlagsInitHandler);
