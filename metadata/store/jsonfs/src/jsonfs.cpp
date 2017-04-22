// Copyright 2017 Stefano Pogliani <stefano@spogliani.net>
#include "ext/metadata/store/jsonfs.h"

#include <fstream>
#include <string>

#include "core/context/context.h"
#include "core/model/logger.h"


using nlohmann::json;
using poolqueue::Promise;

using sf::core::context::ProxyLogger;
using sf::ext::metadata::JsonFsStore;


static ProxyLogger logger("ext.metadata.store.jsonfs");


JsonFsStore::JsonFsStore(std::string store) {
  this->store_ = store;
}


Promise JsonFsStore::cache() {
  // Create a promise to load the store file in cache.
  Promise load_file([this]() {
    this->cache_ = json::object();
    std::ifstream source(this->store_);

    // If the store file is not empty load it.
    if (source.peek() != std::ifstream::traits_type::eof()) {
      source >> this->cache_;
    }
    return this->cache_;
  });

  // Return the cache if loaded or load from file.
  return Promise().settle().then([this, load_file]() {
    if (this->cache_.is_null()) {
      return load_file.settle();
    }
    return Promise().settle(this->cache_);
  });
}

void JsonFsStore::commitCache() {
  std::ofstream store(this->store_);
  store << this->cache_;
  store.close();
}


Promise JsonFsStore::erase(std::string key) {
  return this->cache().then([this, key]() {
    // Remove the key from the cache.
    this->cache_.erase(key);
    this->commitCache();
    return nullptr;
  });
}

Promise JsonFsStore::get(std::string key) {
  return this->cache().then([key](json store) {
    return store[key];
  });
}

Promise JsonFsStore::set(std::string key, json value) {
  return this->cache().then([this, key, value]() {
    // Add value to the store and write the cache back to disk.
    this->cache_[key] = value;
    this->commitCache();
    return nullptr;
  });
}

Promise JsonFsStore::set(
    std::string key, json value,
    std::chrono::duration<int> ttl
) {
  // Log warning about TTL not supported.
  WARNING(logger, "JSONFS metadata store does not support TTL");
  return this->set(key, value);
}
