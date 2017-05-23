// Copyright 2017 Stefano Pogliani <stefano@spogliani.net>
#include "ext/metadata/store/jsonfs/config.h"

#include <string>
#include <vector>

#include "core/cluster/cluster.h"
#include "core/context/context.h"
#include "core/context/static.h"
#include "core/exceptions/configuration.h"

#include "core/interface/config/node.h"
#include "core/interface/config/node/hooks.h"
#include "core/interface/metadata/store.h"
#include "core/interface/posix.h"

#include "core/utility/lua.h"

#include "ext/metadata/store/jsonfs.h"


using sf::core::cluster::Cluster;
using sf::core::cluster::ClusterRaw;

using sf::core::context::Static;
using sf::core::context::ContextRef;
using sf::core::exception::InvalidConfiguration;
using sf::core::hook::NodeConfig;

using sf::core::interface::MetaDataStoreRef;
using sf::core::interface::NodeConfigIntent;
using sf::core::interface::NodeConfigIntentRef;
using sf::core::interface::NodeConfigIntentLuaProxy;
using sf::core::interface::Posix;

using sf::core::utility::Lua;
using sf::core::utility::LuaArguments;
using sf::core::utility::LuaTable;

using sf::ext::metadata::JsonFsStore;
using sf::ext::metadata::JsonFsStoreConfig;

using sf::core::exception::ErrNoException;

//! Verify and configure JsonFsStore.
class JsonFsStoreIntent : public NodeConfigIntent {
 protected:
  std::string path_;

 public:
  JsonFsStoreIntent(std::string path) : NodeConfigIntent("jsonfs") {
    this->path_ = path;
  }

  virtual std::vector<std::string> depends() const {
    return std::vector<std::string>();
  }

  virtual std::string provides() const {
    return "core.metadata";
  }

  virtual void apply(ContextRef context) {
    MetaDataStoreRef store = std::make_shared<JsonFsStore>(this->path_);
    context->initialise(store);
  }

  virtual void verify(ContextRef context) {
    struct stat file_stats;
    auto posix = Static::posix();
    int file_exists = 0;

    // Test path is not a directory or does not exit.
    file_exists = posix->stat(this->path_.c_str(), &file_stats);
    if (file_exists == 0 && S_ISDIR(file_stats.st_mode)) {
      throw InvalidConfiguration(
          "JsonFS store should be a file, got a directory"
      );
    }

    // Test path exists (at least directory).
    char* path = strdup(this->path_.c_str());
    char* dir  = posix->dirname(path);
    file_exists = posix->stat(dir, &file_stats);
    free(path);
    if (file_exists < 0 || !S_ISDIR(file_stats.st_mode)) {
      throw InvalidConfiguration(
          "JsonFS store dirname does not exist or is not a directory"
      );
    }
  }
};


// TODO(stefano): Remove this as soon as the config refactoring is done.
class JsonFsClusterStoreIntent : public JsonFsStoreIntent {
 public:
  JsonFsClusterStoreIntent(std::string path) : JsonFsStoreIntent(path) {
    // NOOP
  }
  virtual std::string provides() const {
    return "cluster.metadata";
  }

  virtual void apply(ContextRef context) {
    MetaDataStoreRef store = std::make_shared<JsonFsStore>(this->path_);
    Cluster cluster = std::make_shared<ClusterRaw>(store);
    context->initialise(cluster);
  }
};


//! Returns a NodeConfigIntent to build a JsonFsStore.
int lua_jsonfs_intent(lua_State* state) {
  NodeConfigIntentLuaProxy type;
  Lua* lua = Lua::fetchFrom(state);

  // Process argument.
  LuaArguments args(lua);
  LuaTable options = args.table(1);
  std::string path = options.toString("store");

  // Create and return the intent.
  auto intent = JsonFsStoreConfig::MakeIntent(path);
  type.wrap(*lua, intent);
  return 1;
}

//! Returns a NodeConfigIntent to build a JsonFsStore for cluster metadata.
// TODO(stefano): Remove this as soon as the config refactoring is done.
int lua_jsonfs_cluster_intent(lua_State* state) {
  NodeConfigIntentLuaProxy type;
  Lua* lua = Lua::fetchFrom(state);

  // Process argument.
  LuaArguments args(lua);
  LuaTable options = args.table(1);
  std::string path = options.toString("store");

  // Create and return the intent.
  auto intent = std::make_shared<JsonFsClusterStoreIntent>(path);
  type.wrap(*lua, intent);
  return 1;
}


void JsonFsStoreConfig::AttachLuaInit() {
  NodeConfig::LuaInit.attach([](Lua lua) {
    auto globals = lua.globals();
    auto metastores = globals->toTable("metastores");
    metastores.set("jsonfs", lua_jsonfs_intent);

    // TODO(stefano): Remove this as soon as the config refactoring is done.
    metastores.set("jsonfs_cluster", lua_jsonfs_cluster_intent);
  });
}

NodeConfigIntentRef JsonFsStoreConfig::MakeIntent(std::string path) {
  return std::make_shared<JsonFsStoreIntent>(path);
}
