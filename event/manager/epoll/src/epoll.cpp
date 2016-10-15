// Copyright 2016 Stefano Pogliani <stefano@spogliani.net>
#include "ext/event/manager/epoll.h"

#include <string>

#include "core/context/context.h"
#include "core/context/static.h"

#include "core/exceptions/base.h"
#include "core/exceptions/event.h"
#include "core/model/logger.h"

#include "core/utility/string.h"

using sf::core::context::Context;
using sf::core::context::Static;
using sf::core::exception::ErrNoException;
using sf::core::exception::EventSourceNotFound;

using sf::core::model::EventRef;
using sf::core::model::EventSourceRef;
using sf::core::model::LogInfo;

using sf::core::utility::string::toString;
using sf::ext::event::EpollLoopManager;


EpollLoopManager::EpollLoopManager() {
  this->epoll_fd = Static::posix()->epoll_create();
}

EpollLoopManager::~EpollLoopManager() {
  Static::posix()->close(this->epoll_fd, true);
}


void EpollLoopManager::add(EventSourceRef source) {
  struct epoll_event event;
  int fd = source->getFD();
  event.data.fd = fd;
  event.events  = EPOLLIN | EPOLLRDHUP | EPOLLPRI | EPOLLERR | EPOLLHUP;

  Static::posix()->epoll_control(this->epoll_fd, EPOLL_CTL_ADD, fd, &event);

  this->sources[source->id()] = source;
  this->sourcesIndex[fd] = source;
}

void EpollLoopManager::removeDrain(std::string id) {
  // NOOP
}

void EpollLoopManager::removeSource(std::string id) {
  if (this->sources.find(id) == this->sources.end()) {
    throw EventSourceNotFound(id);
  }

  EventSourceRef source = this->sources.at(id);
  int fd = source->getFD();

  try {
    Static::posix()->epoll_control(this->epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
  } catch (ErrNoException& ex) {
    if (ex.getCode() != EBADF) {
      throw;
    }
    // Ignore bad file descriptors only.
  }

  this->sourcesIndex.erase(fd);
  this->sources.erase(id);
}

EventRef EpollLoopManager::wait(int timeout) {
  struct epoll_event event = {0};
  int code = Static::posix()->epoll_wait(this->epoll_fd, &event, 1, timeout);

  int fd = event.data.fd;
  if (code == 0) {
    DEBUG(Context::Logger(), "Epoll wait timeout");
    return EventRef();
  }

  if (this->sourcesIndex.find(fd) == this->sourcesIndex.end()) {
    LogInfo vars = {{"source", toString(fd)}};
    ERRORV(Context::Logger(), "Unable to find source for FD ${source}.", vars);
    return EventRef();
  }

  return this->sourcesIndex[fd]->parse();
}
