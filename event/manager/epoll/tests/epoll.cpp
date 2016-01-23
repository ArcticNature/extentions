// Copyright 2016 Stefano Pogliani <stefano@spogliani.net>
#include <fcntl.h>
#include <unistd.h>

#include <gtest/gtest.h>

#include "core/context/static.h"
#include "core/interface/posix.h"

#include "ext/event/manager/epoll.h"


using sf::core::context::Static;
using sf::core::interface::Posix;

using sf::core::model::Event;
using sf::core::model::EventRef;
using sf::core::model::EventSource;
using sf::core::model::EventSourceRef;

using sf::ext::event::EpollSourceManager;


class EpollPosix : public Posix {
 public:
  bool pass_through = false;

  bool created = false;
  bool closed  = false;

  int epoll_op = -1;
  int epoll_fd = -1;

  int close(int fd, bool silent = false) {
    this->closed = true;
    if (this->pass_through) {
      return Posix::close(fd, silent);
    }
    return 0;
  }

  int epoll_control(int epfd, int op, int fd, struct epoll_event* event) {
    this->epoll_op = op;
    this->epoll_fd = fd;

    if (this->pass_through) {
      return Posix::epoll_control(epfd, op, fd, event);
    }
    EXPECT_EQ(1, epfd);
  }

  int epoll_create(int flags = 0) {
    this->created = true;
    if (this->pass_through) {
      return Posix::epoll_create(flags);
    }
    return 1;
  }
};


class EpollSource : public EventSource {
 public:
  EpollSource() : EventSource("epoll-test-source") {}
  int getFD() {
    return 2;
  }
  EventRef parse() {
    return EventRef();
  }
};


class EpollTest : public ::testing::Test {
 protected:
  EpollPosix* posix;

  EpollTest() {
    this->posix = new EpollPosix;
    Static::initialise(this->posix);
  }

  ~EpollTest() {
    Static::reset();
  }
};


class TestEvent : public Event {
 public:
   std::string message;
   TestEvent(std::string correlation) : Event(correlation, "NULL") {}

   void handle() {
     EXPECT_EQ("test", this->message);
   }
};


class PipeSource : public EventSource {
 protected:
  int read_fd;

 public:
  PipeSource(int pipe[2]) : EventSource("test-pipe-source") {
    this->read_fd = pipe[0];
  }
  ~PipeSource() {
    close(this->read_fd);
  }

  int getFD() {
    return this->read_fd;
  }

  EventRef parse() {
    char buffer[5];
    ::read(this->read_fd, buffer, 5);

    TestEvent* event = new TestEvent("cor-1");
    event->message = std::string(buffer);
    return EventRef(event);
  }
};


TEST_F(EpollTest, Add) {
  EpollSourceManager manager;
  EventSourceRef source(new EpollSource());

  manager.addSource(source);
  ASSERT_EQ(EPOLL_CTL_ADD, this->posix->epoll_op);
  ASSERT_EQ(2, this->posix->epoll_fd);
}

TEST_F(EpollTest, Close) {
  {
    // Create and destory the manager.
    EpollSourceManager manager;
  }
  ASSERT_TRUE(this->posix->closed);
}

TEST_F(EpollTest, Create) {
  EpollSourceManager manager;
  ASSERT_TRUE(this->posix->created);
}

TEST_F(EpollTest, Wait) {
  int pipefd[2];
  ASSERT_NE(-1, pipe2(pipefd, O_NONBLOCK));
  write(pipefd[1], "test", 5);
  this->posix->pass_through = true;

  EpollSourceManager manager;
  EventSourceRef source(new PipeSource(pipefd));
  
  manager.addSource(source);
  EventRef event = manager.wait();
  ASSERT_NE(nullptr, event.get());

  event->handle();
  close(pipefd[1]);
}
