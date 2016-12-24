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
using sf::core::exception::EventDrainNotFound;
using sf::core::exception::EventSourceNotFound;

using sf::core::model::EventDrainRef;
using sf::core::model::EventRef;
using sf::core::model::EventSourceRef;
using sf::core::model::LogInfo;

using sf::core::utility::string::toString;
using sf::ext::event::EpollLoopManager;


EventRef EpollLoopManager::processBoth(int fd) {
  // Look for and process the drain.
  try {
    this->processDrain(fd);
  } catch (EventDrainNotFound&) {
    LogInfo vars = {{"fd", toString(fd)}};
    DEBUGV(
        Context::Logger(),
        "Skipping error/hup handling for non-drain ${fd}.", vars
    );
  }

  // Look for and process the source.
  try {
    return this->sources.get(fd)->fetch();
  } catch (EventSourceNotFound&) {
    LogInfo vars = {{"fd", toString(fd)}};
    DEBUGV(
        Context::Logger(),
        "Skipping error/hup handling for non-source ${fd}.", vars
    );
    return EventRef();
  }
}

EventRef EpollLoopManager::processSource(int fd) {
  try {
    return this->sources.get(fd)->fetch();
  } catch (EventSourceNotFound&) {
    LogInfo vars = {{"source", toString(fd)}};
    ERRORV(Context::Logger(), "Unable to find source for FD ${source}.", vars);
    return EventRef();
  }
}


EpollLoopManager::EpollLoopManager() {
  this->epoll_fd = Static::posix()->epoll_create();
}

EpollLoopManager::~EpollLoopManager() {
  Static::posix()->close(this->epoll_fd, true);
}


void EpollLoopManager::add(EventDrainRef drain) {
  struct epoll_event event;
  int fd = this->fdFor(drain);
  event.data.fd = fd;
  event.events  = EPOLLOUT | EPOLLHUP | EPOLLERR;

  Static::posix()->epoll_control(this->epoll_fd, EPOLL_CTL_ADD, fd, &event);

  this->drains.add(drain);
}

void EpollLoopManager::add(EventSourceRef source) {
  struct epoll_event event;
  int fd = this->fdFor(source);
  event.data.fd = fd;
  event.events  = EPOLLIN | EPOLLRDHUP | EPOLLPRI | EPOLLERR | EPOLLHUP;

  Static::posix()->epoll_control(this->epoll_fd, EPOLL_CTL_ADD, fd, &event);

  this->sources.add(source);
}

void EpollLoopManager::removeDrain(std::string id) {
  EventDrainRef drain = this->drains.get(id);
  int fd = this->fdFor(drain);

  try {
    Static::posix()->epoll_control(this->epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
  } catch (ErrNoException& ex) {
    // Ignore bad file descriptors only.
    if (ex.getCode() != EBADF) {
      this->drains.remove(id);
      throw;
    }
  }

  this->drains.remove(id);
}

void EpollLoopManager::removeSource(std::string id) {
  EventSourceRef source = this->sources.get(id);
  int fd = this->fdFor(source);

  try {
    Static::posix()->epoll_control(this->epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
  } catch (ErrNoException& ex) {
    // Ignore bad file descriptors only.
    if (ex.getCode() != EBADF) {
      this->sources.remove(id);
      throw;
    }
  }

  this->sources.remove(id);
}

EventRef EpollLoopManager::wait(int timeout) {
  struct epoll_event event = {0};
  int code = Static::posix()->epoll_wait(this->epoll_fd, &event, 1, timeout);

  int fd = event.data.fd;
  if (code == 0) {
    DEBUG(Context::Logger(), "Epoll wait timeout");
    return EventRef();
  }

  // Is it an input event?
  if (event.events & (EPOLLIN | EPOLLRDHUP | EPOLLPRI)) {
    return this->processSource(fd);
  }

  // Is it an output event?
  if (event.events & EPOLLOUT) {
    this->processDrain(fd);
    return EventRef();
  }

  // Is it an error or an hup?
  if (event.events & (EPOLLHUP | EPOLLERR)) {
    return this->processBoth(fd);
  }

  // Unkown event.
  return EventRef();
}
