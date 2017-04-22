// Copyright 2017 Stefano Pogliani <stefano@spogliani.net>
#ifndef EXT_METADATA_STORE_JSONFS_H_
#define EXT_METADATA_STORE_JSONFS_H_

#include <string>

#include "core/interface/metadata/store.h"


namespace sf {
namespace ext {
namespace metadata {

  //! Metadata store backed by JSON files on the local machine.
  /*!
   * Stores all Key/Value pairs in one JSON file with:
   *
   *    {
   *      "<key>": <value>,
   *      ...
   *    }
   */
  class JsonFsStore : public sf::core::interface::MetaDataStore {
   protected:
    std::string store_;
    nlohmann::json cache_;

    //! Returns (loading from file if needed) the JSON cache.
    poolqueue::Promise cache();

    //! Helper method that writes the current cache to file.
    void commitCache();

   public:
    explicit JsonFsStore(std::string store);

    poolqueue::Promise erase(std::string key);
    poolqueue::Promise get(std::string key);
    poolqueue::Promise set(std::string key, nlohmann::json value);
    poolqueue::Promise set(
        std::string key, nlohmann::json value,
        std::chrono::duration<int> ttl
    );
  };

}  // namespace metadata
}  // namespace ext
}  // namespace sf

#endif  // EXT_METADATA_STORE_JSONFS_H_
