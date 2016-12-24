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
  class EpollLoopManager : public sf::core::model::LoopManager {
   protected:
    int epoll_fd;

    //! Process an event on both a drain and a source.
    sf::core::model::EventRef processBoth(int fd);

    //! Process an event on a source.
    sf::core::model::EventRef processSource(int fd);

   public:
    EpollLoopManager();
    virtual ~EpollLoopManager();

    void add(sf::core::model::EventDrainRef source);
    void add(sf::core::model::EventSourceRef source);
    void removeDrain(std::string id);
    void removeSource(std::string id);
    sf::core::model::EventRef wait(int timeout = -1);
  };

}  // namespace event
}  // namespace ext
}  // namespace sf

#endif  // EXT_EVENT_MANAGER_EPOLL_H_
