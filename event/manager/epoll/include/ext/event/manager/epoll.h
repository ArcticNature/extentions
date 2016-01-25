// Copyright 2016 Stefano Pogliani <stefano@spogliani.net>
#ifndef EXT_EVENT_MANAGER_EPOLL_H_
#define EXT_EVENT_MANAGER_EPOLL_H_

#include <map>
#include <string>

#include "core/model/event.h"


namespace sf {
namespace ext {
namespace event {

  //! Epoll based EventSourceManager implementation
  class EpollSourceManager : public sf::core::model::EventSourceManager {
   protected:
    int epoll_fd;
    std::map<int, sf::core::model::EventSourceRef> index;

   public:
    EpollSourceManager();
    virtual ~EpollSourceManager();

    void addSource(sf::core::model::EventSourceRef source);
    void removeSource(std::string id);
    sf::core::model::EventRef wait(int timeout = -1);
  };

}  // namespace event
}  // namespace ext
}  // namespace sf

#endif  // EXT_EVENT_MANAGER_EPOLL_H_
