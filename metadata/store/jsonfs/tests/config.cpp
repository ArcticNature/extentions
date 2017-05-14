// Copyright 2017 Stefano Pogliani <stefano@spogliani.net>
#include <gtest/gtest.h>

#include "core/context/context.h"
#include "core/context/static.h"
#include "core/exceptions/configuration.h"
#include "core/exceptions/lua.h"

#include "core/interface/config/node.h"
#include "core/interface/config/node/hooks.h"
#include "core/interface/posix.h"
#include "core/utility/lua.h"

#include "ext/metadata/store/jsonfs/config.h"

#include "core/testing/hooks.h"


using sf::core::context::Context;
using sf::core::context::ContextRef;
using sf::core::context::Static;

using sf::core::exception::InvalidConfiguration;
using sf::core::exception::LuaRuntimeError;
using sf::core::exception::LuaTypeError;

using sf::core::hook::NodeConfig;
using sf::core::interface::MetaDataStoreRef;
using sf::core::interface::NodeConfigIntent;
using sf::core::interface::NodeConfigIntentLuaProxy;
using sf::core::interface::Posix;
using sf::core::utility::Lua;

using sf::ext::metadata::JsonFsStoreConfig;

using sf::core::testing::HookTest;


class ConfigExtensionTest : public HookTest {
 public:
  Lua lua;
  NodeConfigIntentLuaProxy type;

  ConfigExtensionTest() : lua(), type(lua) {
    this->trackHook(&NodeConfig::LuaInit);
    this->trackHook(&NodeConfig::Collect);
    this->clearHooks();

    this->type.initType(this->lua);
    this->lua.doString("metastores = {}");
  }

  ~ConfigExtensionTest() {
    this->clearHooks();
  }
};


TEST_F(ConfigExtensionTest, FactoryIsFunction) {
  // Attach and trigger LuaInit handler.
  JsonFsStoreConfig::AttachLuaInit();
  NodeConfig::LuaInit.trigger(this->lua);

  // Check that `metastores.jsonfs` is a function.
  this->lua.doString("return metastores.jsonfs");
  ASSERT_EQ(LUA_TFUNCTION, this->lua.stack()->type());
}

TEST_F(ConfigExtensionTest, FactoryReturnsIntent) {
  // Attach and trigger LuaInit handler.
  JsonFsStoreConfig::AttachLuaInit();
  NodeConfig::LuaInit.trigger(this->lua);

  // Invoke factory and check return type.
  this->lua.doString(
      "return metastores.jsonfs {store = '/some/path'}"
  );
  ASSERT_TRUE(this->type.typeOf(-1));
}

TEST_F(ConfigExtensionTest, FactoryRequiresArg) {
  // Attach and trigger LuaInit handler.
  JsonFsStoreConfig::AttachLuaInit();
  NodeConfig::LuaInit.trigger(this->lua);

  // Invoke factory and check return type.
  ASSERT_THROW(
      this->lua.doString("return metastores.jsonfs()"),
      LuaRuntimeError
  );
}

TEST_F(ConfigExtensionTest, FactoryRequiresStorePath) {
  // Attach and trigger LuaInit handler.
  JsonFsStoreConfig::AttachLuaInit();
  NodeConfig::LuaInit.trigger(this->lua);

  // Invoke factory and check return type.
  ASSERT_THROW(
      this->lua.doString("return metastores.jsonfs {}"),
      LuaTypeError
  );
}


class JsonFsStoreIntentTest : public ::testing::Test {
 public:
  JsonFsStoreIntentTest() {
    Static::initialise(new Posix());
  }

  ~JsonFsStoreIntentTest() {
    Static::destroy();
  }
};


TEST_F(JsonFsStoreIntentTest, UpdatesTheContext) {
  auto intent = JsonFsStoreConfig::MakeIntent("");
  ContextRef context(new Context());
  intent->apply(context);
  ASSERT_NO_THROW(context->metadata());
}

TEST_F(JsonFsStoreIntentTest, VaildatePathIsDir) {
  auto intent = JsonFsStoreConfig::MakeIntent("/");
  ContextRef context(new Context());
  ASSERT_THROW(intent->verify(context), InvalidConfiguration);
}

TEST_F(JsonFsStoreIntentTest, VaildatePathIsMissing) {
  auto intent = JsonFsStoreConfig::MakeIntent("/not/a/real/path");
  ContextRef context(new Context());
  ASSERT_THROW(intent->verify(context), InvalidConfiguration);
}

TEST_F(JsonFsStoreIntentTest, VaildatePathIsValid) {
  auto intent = JsonFsStoreConfig::MakeIntent("/tmp/store");
  ContextRef context(new Context());
  ASSERT_NO_THROW(intent->verify(context));
}
