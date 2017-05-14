// Copyright 2017 Stefano Pogliani <stefano@spogliani.net>
#include <string>

#include "core/interface/lifecycle.h"
#include "ext/metadata/store/jsonfs/config.h"


using sf::core::interface::BaseLifecycleArg;
using sf::core::interface::BaseLifecycleHandler;

using sf::ext::metadata::JsonFsStoreConfig;


class JsonFSModuleInit : public BaseLifecycleHandler {
 public:
  void handle(std::string event, BaseLifecycleArg* arg) {
    // Attach to LuaInit hook.
    JsonFsStoreConfig::AttachLuaInit();
  }
};


LifecycleStaticOn("process::init", JsonFSModuleInit);
