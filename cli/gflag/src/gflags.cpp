// Copyright 2015 Stefano Pogliani <stefano@spogliani.net>
#include "ext/cli/gflags.h"

#include <gflags/gflags.h>

#include <map>
#include <string>
#include <vector>

#include "core/model/options.h"


using sf::core::model::CLIOptionRef;
using sf::ext::cli::GFlagsCLI;


DEFINE_string(parser, "gflags", "Specify the command line parser to use.");

// Daemon options.
DEFINE_bool(daemonise, true, "Run the process in daemon mode");
DEFINE_bool(
    drop_privileges, false,
    "Drop prividelges even when not in daemon mode"
);
DEFINE_string(group,   "snow-fox", "Group for the daemon to run as");
DEFINE_string(user,    "snow-fox", "User for the daemon to run as");

DEFINE_string(stderr, "", "Redirect stderr to this file");
DEFINE_string(stdin,  "", "Redirect stdin to this file");
DEFINE_string(stdout, "", "Redirect stdout to this file");
DEFINE_string(work_dir, ".", "Change the working directory for the process");

// Cluster options.
DEFINE_string(node_name, "", "Unique name of the node");

// Configuration repository options.
DEFINE_string(repo_type, "git", "The type of configuration repository");
DEFINE_string(repo_path, "", "The path to the configuration repository");
DEFINE_string(repo_ver,  "<latest>", "Version of the configuration to use");

// Socket options.
DEFINE_string(
    manager_socket,
    "/var/run/snow-fox-manager.socket",
    "Path to the socket file to connect daemon and manager."
);
DEFINE_string(
    spawner_socket,
    "/var/run/snow-fox-spawner.socket",
    "Path to the socket file to connect daemon and spawner."
);
DEFINE_string(
    spawner_manager_socket,
    "/var/run/snow-fox-manager-spawner.socket",
    "Path to the socket file to connect manager and spawner."
);

// Misc options.
DEFINE_bool(Version, false, "Show version and exit");


void GFlagsCLI::optionalSetString(std::string name, std::string value) {
  if (value != "") {
    this->setString(name, value);
  }
}

void GFlagsCLI::parseLogic(int* argc, char*** argv) {
  ::google::ParseCommandLineFlags(argc, argv, true);

  // Build values maps.
  std::map<std::string, bool> bools = {
    {"daemonise", FLAGS_daemonise},
    {"drop-privileges", FLAGS_drop_privileges},
    {"version", FLAGS_Version}
  };

  std::map<std::string, std::string> optional_strings = {
    {"group", FLAGS_group},
    {"user", FLAGS_user},
    {"work-dir", FLAGS_work_dir},

    {"stderr", FLAGS_stderr},
    {"stdin",  FLAGS_stdin},
    {"stdout", FLAGS_stdout},

    {"node-name", FLAGS_node_name},

    {"repo-path", FLAGS_repo_path},
    {"repo-type", FLAGS_repo_type},
    {"repo-version", FLAGS_repo_ver},

    {"manager-socket", FLAGS_manager_socket},
    {"spawner-socket", FLAGS_spawner_socket},
    {"spawner-manager-socket", FLAGS_spawner_manager_socket}
  };

  // Populate added options based on maps.
  for (std::vector<CLIOptionRef>::iterator it = this->options.begin();
      it != this->options.end(); it++) {
    std::string name = (*it)->name();
    if (bools.find(name) != bools.end()) {
      this->setBoolean(name, bools[name]);
    }
    if (optional_strings.find(name) != optional_strings.end()) {
      this->optionalSetString(name, optional_strings[name]);
    }
  }
}
