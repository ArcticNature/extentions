// Copyright 2015 Stefano Pogliani <stefano@spogliani.net>
#include "ext/cli/gflags.h"

#include <gflags/gflags.h>
#include <string>


using sf::ext::cli::GFlagsCLI;


DEFINE_string(parser, "gflags", "Specify the command line parser to use.");

// Daemon options.
DEFINE_bool(daemonise, true, "Run the process in daemon mode");
DEFINE_string(group,   "snow-fox", "Group for the daemon to run as");
DEFINE_string(user,    "snow-fox", "User for the daemon to run as");

DEFINE_string(stderr, "", "Redirect stderr to this file");
DEFINE_string(stdin,  "", "Redirect stdin to this file");
DEFINE_string(stdout, "", "Redirect stdout to this file");
DEFINE_string(work_dir, ".", "Change the working directory for the process");

// Configuration repository options.
DEFINE_string(repo_type, "git", "The type of configuration repository");
DEFINE_string(repo_path, "", "The path to the configuration repository");
DEFINE_string(repo_ver,  "<latest>", "Version of the configuration to use");


void GFlagsCLI::optionalSetString(std::string name, std::string value) {
  if (value != "") {
    this->setString(name, value);
  }
}

void GFlagsCLI::parseLogic(int* argc, char*** argv) {
  ::google::ParseCommandLineFlags(argc, argv, true);

  // Daemon options.
  this->setBoolean("daemonise", FLAGS_daemonise);
  this->optionalSetString("group", FLAGS_group);
  this->optionalSetString("user", FLAGS_user);
  this->optionalSetString("work-dir", FLAGS_work_dir);

  this->optionalSetString("stderr", FLAGS_stderr);
  this->optionalSetString("stdin",  FLAGS_stdin);
  this->optionalSetString("stdout", FLAGS_stdout);

  // Repository options.
  this->optionalSetString("repo-path", FLAGS_repo_path);
  this->optionalSetString("repo-type", FLAGS_repo_type);
  this->optionalSetString("repo-version", FLAGS_repo_ver);
}
