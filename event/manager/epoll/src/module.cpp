// Copyright 2015 Stefano Pogliani <stefano@spogliani.net>
#include <string>

#include "core/interface/lifecycle.h"
#include "core/registry/event/managers.h"
#include "ext/event/manager/epoll.h"

using sf::core::interface::BaseLifecycleArg;
using sf::core::interface::BaseLifecycleHandler;
using sf::core::interface::Lifecycle;
using sf::core::interface::LifecycleHandlerRef;

using sf::core::model::EventSourceManagerRef;
using sf::core::registry::EventSourceManager;
using sf::ext::event::EpollSourceManager;


EventSourceManagerRef epoll_factory() {
  return EventSourceManagerRef(new EpollSourceManager());
}


//! Module initialiser for the EPoll source manager module.
class EventManagerEpollInitHandler : public BaseLifecycleHandler {
 public:
  void handle(std::string event, BaseLifecycleArg*) {
    EventSourceManager::RegisterFactory("epoll", epoll_factory);
  }
};


// Module initialiser.
class EventManagerEpollModuleInit {
 public:
  EventManagerEpollModuleInit() {
    LifecycleHandlerRef handler(new EventManagerEpollInitHandler());
    Lifecycle::on("process::init", handler);
  }
};
EventManagerEpollModuleInit event_manager_epoll_module;
