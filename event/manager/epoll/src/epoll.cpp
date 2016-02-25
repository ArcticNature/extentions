// Copyright 2016 Stefano Pogliani <stefano@spogliani.net>
#include "ext/event/manager/epoll.h"

#include <string>

#include "core/context/context.h"
#include "core/context/static.h"

#include "core/exceptions/event.h"
#include "core/model/logger.h"

#include "core/utility/string.h"

using sf::core::context::Context;
using sf::core::context::Static;
using sf::core::exception::EventSourceNotFound;

using sf::core::model::EventRef;
using sf::core::model::EventSourceRef;
using sf::core::model::LogInfo;

using sf::core::utility::string::toString;
using sf::ext::event::EpollSourceManager;


EpollSourceManager::EpollSourceManager() {
  this->epoll_fd = Static::posix()->epoll_create();
}

EpollSourceManager::~EpollSourceManager() {
  Static::posix()->close(this->epoll_fd, true);
}


void EpollSourceManager::addSource(EventSourceRef source) {
  struct epoll_event event;
  int fd = source->getFD();
  event.data.fd = fd;
  event.events  = EPOLLIN | EPOLLRDHUP | EPOLLPRI | EPOLLERR | EPOLLHUP;

  Static::posix()->epoll_control(this->epoll_fd, EPOLL_CTL_ADD, fd, &event);

  this->sources[source->id()] = source;
  this->index[fd] = source;
}

void EpollSourceManager::removeSource(std::string id) {
  if (this->sources.find(id) == this->sources.end()) {
    throw EventSourceNotFound(id);
  }

  EventSourceRef source = this->sources.at(id);
  int fd = source->getFD();

  Static::posix()->epoll_control(this->epoll_fd, EPOLL_CTL_DEL, fd, nullptr);

  this->index.erase(fd);
  this->sources.erase(id);
}

EventRef EpollSourceManager::wait(int timeout) {
  struct epoll_event event = {0};
  int code = Static::posix()->epoll_wait(this->epoll_fd, &event, 1, timeout);

  int fd = event.data.fd;
  if (code == 0) {
    DEBUG(Context::logger(), "Epoll wait timeout");
    return EventRef();
  }

  if (this->index.find(fd) == this->index.end()) {
    LogInfo vars = {{"source", toString(fd)}};
    ERRORV(Context::logger(), "Unable to find source for FD ${source}.", vars);
    return EventRef();
  }

  return this->index[fd]->parse();
}
