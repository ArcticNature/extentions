// Copyright 2015 Stefano Pogliani <stefano@spogliani.net>
#include <string>
#include <vector>

#include "core/context/context.h"
#include "core/interface/config/node.h"
#include "core/interface/lifecycle.h"

#include "core/model/logger.h"
#include "core/registry/event/managers.h"

#include "core/utility/lua.h"
#include "ext/event/manager/epoll.h"

using sf::core::context::Context;
using sf::core::context::ContextRef;

using sf::core::interface::BaseLifecycleArg;
using sf::core::interface::BaseLifecycleHandler;
using sf::core::interface::Lifecycle;
using sf::core::interface::LifecycleHandlerRef;
using sf::core::interface::NodeConfigIntent;
using sf::core::interface::NodeConfigIntentLuaProxy;

using sf::core::lifecycle::NodeConfigLifecycleArg;
using sf::core::lifecycle::NodeConfigLifecycleHandler;

using sf::core::model::LoopManagerRef;
using sf::core::registry::LoopManager;

using sf::core::utility::Lua;
using sf::core::utility::LuaTable;
using sf::ext::event::EpollLoopManager;


class EpollConfigIntent : public NodeConfigIntent {
 protected:
  static const std::vector<std::string> DEPENDS;

 public:
  EpollConfigIntent() : NodeConfigIntent("event_manager.epoll") {
    // NOOP.
  }

  std::vector<std::string> depends() const {
    return EpollConfigIntent::DEPENDS;
  }

  std::string provides() const {
    return "event.manager";
  }

  void apply(ContextRef context) {
    context->initialise(LoopManagerRef(new EpollLoopManager()));
  }

  void verify(ContextRef context) {
    // NOOP.
  }
};
const std::vector<std::string> EpollConfigIntent::DEPENDS = {};


LoopManagerRef epoll_factory() {
  return LoopManagerRef(new EpollLoopManager());
}


int lua_epoll_node_config_intent(lua_State* state) {
  Lua* lua = Lua::fetchFrom(state);
  NodeConfigIntentLuaProxy type;
  type.wrap(*lua, new EpollConfigIntent());
  return 1;
}


//! Module initialiser for the EPoll source manager module.
class LoopManEpollProcessInit : public BaseLifecycleHandler {
 public:
  void handle(std::string event, BaseLifecycleArg*) {
    LoopManager::RegisterFactory("epoll", epoll_factory);
  }
};


class LoopManEpollConfNodeLuaInit : public NodeConfigLifecycleHandler {
 public:
  void handle(std::string event, NodeConfigLifecycleArg* arg) {
    Lua* lua = arg->lua();
    LuaTable event_managers = lua->globals()->toTable("event_managers");
    lua->stack()->push(lua_epoll_node_config_intent, 0);
    event_managers.fromStack("epoll");
    DEBUG(Context::Logger(), "Registered NodeConfig::event_managers.epoll");
  }
};


// Module initialiser.
LifecycleStaticOn("process::init", LoopManEpollProcessInit);
LifecycleStaticOn("config::node::init-lua", LoopManEpollConfNodeLuaInit);
