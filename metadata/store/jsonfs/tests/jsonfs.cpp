// Copyright 2017 Stefano Pogliani <stefano@spogliani.net>
#include <gtest/gtest.h>

#include <stdlib.h>
#include <fstream>

#include "ext/metadata/store/jsonfs.h"


using nlohmann::json;
using poolqueue::Promise;

using sf::core::interface::MetaDataStoreRef;
using sf::ext::metadata::JsonFsStore;


void ASSERT_PROMISE_NO_THROW(Promise promise) {
  promise.except([](const std::exception_ptr& exception) {
    try {
      std::rethrow_exception(exception);
    } catch (std::exception& ex) {
      ADD_FAILURE() << "Promise throws -- " << ex.what();
    }
    return nullptr;
  });
}


class JsonFsStoreTest : public ::testing::Test {
 protected:
  int tmp_fd_;
  std::string tmp_path_;
  MetaDataStoreRef store;

 public:
  JsonFsStoreTest() {
    char* path = strdup("tmp.sf-jsonfs.tests.XXXXXX.json");
    this->tmp_fd_ = mkstemp(path);
    this->tmp_path_ = std::string(path);
    this->store = std::make_shared<JsonFsStore>(this->tmp_path_);
  }

  ~JsonFsStoreTest() {
    this->store.reset();
    close(this->tmp_fd_);
  }

  json loadStore() {
    std::ifstream file(this->tmp_path_);
    json data;
    file >> data;
    return data;
  }
};


TEST_F(JsonFsStoreTest, EraseExistingKeyUpdatesFile) {
  // Store some data in our file.
  std::ofstream file(this->tmp_path_);
  json data = {
    {"key", "value"}
  };
  file << data;
  file.close();

  // Erease an existing key.
  auto erase = this->store->erase("key");
  ASSERT_PROMISE_NO_THROW(erase);
  ASSERT_TRUE(erase.settled());

  // Read the json back to check the content.
  auto store = this->loadStore();
  ASSERT_EQ(0, store.size());
}

TEST_F(JsonFsStoreTest, GetStoredFile) {
  std::ofstream file(this->tmp_path_);
  json data = {
    {"key", "value"}
  };
  file << data;
  file.close();

  auto value = this->store->get("key").then([](json v) {
    std::string value = v;
    EXPECT_EQ("value", value);
    return nullptr;
  });
  ASSERT_PROMISE_NO_THROW(value);
  ASSERT_TRUE(value.settled());
}

TEST_F(JsonFsStoreTest, StoreJson) {
  // Store something new.
  auto save = this->store->set("key", "\"value\""_json);
  ASSERT_PROMISE_NO_THROW(save);
  ASSERT_TRUE(save.settled());

  // Read the json back to check the content.
  auto data = this->loadStore();
  std::string value = data["key"];
  ASSERT_EQ("value", value);
}

TEST_F(JsonFsStoreTest, StoreThenGet) {
  auto save = this->store->set("test", "42"_json);
  ASSERT_PROMISE_NO_THROW(save);
  ASSERT_TRUE(save.settled());

  auto load = this->store->get("test").then([](json v) {
    int value = v;
    EXPECT_EQ(42, value);
    return nullptr;
  });
  ASSERT_PROMISE_NO_THROW(load);
  ASSERT_TRUE(load.settled());
}

TEST_F(JsonFsStoreTest, StoreWithTTL) {
  std::chrono::duration<int> ttl(0);
  auto save = this->store->set("test", "42"_json, ttl);
  ASSERT_PROMISE_NO_THROW(save);
  ASSERT_TRUE(save.settled());

  auto load = this->store->get("test").then([](json v) {
    int value = v;
    EXPECT_EQ(42, value);
    return nullptr;
  });
  ASSERT_PROMISE_NO_THROW(load);
  ASSERT_TRUE(load.settled());
}
