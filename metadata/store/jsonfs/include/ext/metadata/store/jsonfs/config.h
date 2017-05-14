// Copyright 2017 Stefano Pogliani <stefano@spogliani.net>
#ifndef EXT_METADATA_STORE_JSONFS_CONFIG_H_
#define EXT_METADATA_STORE_JSONFS_CONFIG_H_

#include <string>

#include "core/interface/config/node.h"


namespace sf {
namespace ext {
namespace metadata {

  //! Configuration options for JsonFsStore.
  class JsonFsStoreConfig {
   public:
    static void AttachLuaInit();
    static sf::core::interface::NodeConfigIntentRef MakeIntent(
        std::string path
    );
  };

}  // namespace metadata
}  // namespace ext
}  // namespace sf

#endif  // EXT_METADATA_STORE_JSONFS_CONFIG_H_
