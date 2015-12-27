// Copyright 2015 Stefano Pogliani <stefano@spogliani.net>
#ifndef EXT_CLI_GFLAGS_H_
#define EXT_CLI_GFLAGS_H_

#include <string>

#include "core/model/cli-parser.h"


namespace sf {
namespace ext {
namespace cli {

  //! Google Flags based parser.
  /*!
   * The following flags are supported:
   * --repo_path=PATH The path to the config repo.
   * --repo_type=TYPE The type of repository to use (dir|git).
   *
   * --[no]daemonise Start in daemon mode (or not).
   * --group=GROUP   The group to drop to when daemonised.
   * --user=USER     The user to drop to when daemonised.
   *
   * --stderr=FILE  Redirect stderr to a file.
   * --stdin=FILE   Redirect stdin to a file.
   * --stdout=FILE  Redirect stdout to a file.
   *
   * --work_dir=PATH Change working directory.
   */
  class GFlagsCLI : public sf::core::model::CLIParser {
   protected:
    void optionalSetString(std::string name, std::string value);
    virtual void parseLogic(int* argc, char*** argv);
  };

}  // namespace cli
}  // namespace ext
}  // namespace sf

#endif  // EXT_CLI_GFLAGS_H_
