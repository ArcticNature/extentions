// Copyright 2016 Stefano Pogliani <stefano@spogliani.net>
#include <fcntl.h>
#include <unistd.h>

#include <gtest/gtest.h>

#include "core/context/static.h"
#include "core/interface/posix.h"

#include "ext/event/manager/epoll.h"

#include "core/event/testing.h"


using sf::core::context::Static;
using sf::core::interface::Posix;

using sf::core::model::Event;
using sf::core::model::EventDrain;
using sf::core::model::EventDrainBuffer;
using sf::core::model::EventDrainBufferRef;
using sf::core::model::EventDrainRef;
using sf::core::model::EventRef;
using sf::core::model::EventSource;
using sf::core::model::EventSourceRef;

using sf::ext::event::EpollLoopManager;

using sf::core::event::TestEvent;


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


class EpollDrain : public EventDrain {
 public:
  EpollDrain() : EventDrain("epoll-test-drain") {
    // Noop.
  }

  int fd() {
    return 3;
  }

  bool flush() {
    return true;
  }
};


class EpollSource : public EventSource {
 public:
  EpollSource() : EventSource("epoll-test-source") {
    // Noop.
  }

  int fd() {
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
    Static::destroy();
  }

  void enqueue(EventDrainRef drain, std::string msg) {
    uint32_t size = msg.length();
    EventDrainBufferRef buffer(new EventDrainBuffer(size));
    char* data = static_cast<char*>(buffer->data(0));
    strncpy(data, msg.c_str(), size);
    drain->enqueue(buffer);
  }
};


class EpollTestEvent : public TestEvent {
 public:
   std::string message;
   void handle() {
     EXPECT_EQ("test", this->message);
   }
};


class PipeDrain : public EventDrain {
 protected:
  int write_fd;

 public:
  PipeDrain(int pipe[2]) : EventDrain("test-pipe-drain") {
    this->write_fd = pipe[1];
  }
  ~PipeDrain() {
    close(this->write_fd);
  }

  bool flush() {
    if (this->buffer.empty()) {
      return true;
    }

    uint32_t size;
    EventDrainBufferRef buffer = this->buffer[0];
    this->buffer.erase(this->buffer.begin());
    char* data = buffer->remaining(&size);
    ::write(this->write_fd, data, size);
    return this->buffer.empty();
  }

  int fd() {
    return this->write_fd;
  }
};


class PipeSource : public EventSource {
 protected:
  int read_fd;

  EventRef parse() {
    char buffer[5];
    ::read(this->read_fd, buffer, 5);

    EpollTestEvent* event = new EpollTestEvent();
    event->message = std::string(buffer);
    return EventRef(event);
  }

 public:
  PipeSource(int pipe[2]) : EventSource("test-pipe-source") {
    this->read_fd = pipe[0];
  }
  ~PipeSource() {
    close(this->read_fd);
  }

  int fd() {
    return this->read_fd;
  }
};


TEST_F(EpollTest, AddDrain) {
  EpollLoopManager manager;
  EventDrainRef drain(new EpollDrain());
  manager.add(drain);
  ASSERT_EQ(EPOLL_CTL_ADD, this->posix->epoll_op);
  ASSERT_EQ(3, this->posix->epoll_fd);
}

TEST_F(EpollTest, AddSource) {
  EpollLoopManager manager;
  EventSourceRef source(new EpollSource());
  manager.add(source);
  ASSERT_EQ(EPOLL_CTL_ADD, this->posix->epoll_op);
  ASSERT_EQ(2, this->posix->epoll_fd);
}

TEST_F(EpollTest, Close) {
  {
    // Create and destroy the manager.
    EpollLoopManager manager;
  }
  ASSERT_TRUE(this->posix->closed);
}

TEST_F(EpollTest, Create) {
  EpollLoopManager manager;
  ASSERT_TRUE(this->posix->created);
}

TEST_F(EpollTest, WaitDrain) {
  int pipefd[2];
  ASSERT_NE(-1, pipe2(pipefd, O_NONBLOCK));
  this->posix->pass_through = true;

  EpollLoopManager manager;
  EventDrainRef drain(new PipeDrain(pipefd));
  manager.add(drain);

  this->enqueue(drain, "ABCD");
  EventRef event = manager.wait(1);
  ASSERT_EQ(nullptr, event.get());

  char buffer[50];
  int size = ::read(pipefd[0], buffer, 50);
  std::string msg(buffer, size);
  ASSERT_EQ("ABCD", msg);
}

TEST_F(EpollTest, WaitSource) {
  int pipefd[2];
  ASSERT_NE(-1, pipe2(pipefd, O_NONBLOCK));
  write(pipefd[1], "test", 5);
  this->posix->pass_through = true;

  EpollLoopManager manager;
  EventSourceRef source(new PipeSource(pipefd));
  manager.add(source);
  EventRef event = manager.wait();
  ASSERT_NE(nullptr, event.get());

  event->handle();
  close(pipefd[1]);
}
