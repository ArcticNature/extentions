// Copyright 2015 Stefano Pogliani <stefano@spogliani.net>
#include <gtest/gtest.h>

#include "core/exceptions/base.h"
#include "core/model/cli-parser.h"
#include "ext/cli/gflags.h"


using sf::core::exception::InvalidCommandLine;
using sf::core::model::CLIParser;
using sf::ext::cli::GFlagsCLI;


class GFlagsTest : public ::testing::Test {
 protected:
  void parseVector(CLIParser& parser, std::vector<std::string> args) {
    int    argc = args.size();
    char** argv = new char*[argc];

    // Convert vector to C array.
    for (int idx=0; idx<argc; idx++) {
      const char* source = args[idx].c_str();
      int   length = strlen(source);
      char* result = new char[length + 1];
      strcpy(result, source);
      argv[idx] = result;
    }

    // Freeing argv after GFlags leads to segfaults.
    // Just leak memory and move on.
    parser.parse(&argc, &argv);
  }
};


// The orider of the tests is important due to GFlags internals.
TEST_F(GFlagsTest, NoRepo) {
  GFlagsCLI parser;
  CLIParser::configOptions(&parser);
  CLIParser::daemonOptions(&parser);
  CLIParser::miscOptions(&parser);
  std::vector<std::string> args = { {"/usr/bin/test"} };
  ASSERT_THROW(this->parseVector(parser, args), InvalidCommandLine);
}

TEST_F(GFlagsTest, WithRepo) {
  GFlagsCLI parser;
  CLIParser::configOptions(&parser);
  CLIParser::daemonOptions(&parser);
  CLIParser::miscOptions(&parser);
  std::vector<std::string> args = {
    {"/usr/bin/test"},
    {"--repo_path=/test"}
  };
  
  this->parseVector(parser, args);
  ASSERT_EQ("/test", parser.getString("repo-path"));
  ASSERT_EQ("", parser.getString("stderr"));
}

TEST_F(GFlagsTest, AllFlags) {
  GFlagsCLI parser;
  CLIParser::configOptions(&parser);
  CLIParser::daemonOptions(&parser);
  CLIParser::miscOptions(&parser);
  std::vector<std::string> args = {
    {"/usr/bin/test"}, {"--parser=test"},
    {"--nodaemonise"}, {"--group=g"}, {"--user=u"},
    {"--stderr=e"}, {"--stdin=i"}, {"--stdout=o"},
    {"--work_dir=wd"}, {"--repo_type=rt"},
    {"--repo_path=rp"}, {"--repo_ver=rv"},
    {"--manager_socket=ms"}, {"--spawner_socket=ss"},
    {"--spawner_manager_socket=sms"}
  };

  this->parseVector(parser, args);
  ASSERT_FALSE(parser.getBoolean("daemonise"));
  ASSERT_EQ("g", parser.getString("group"));
  ASSERT_EQ("u", parser.getString("user"));

  ASSERT_EQ("e", parser.getString("stderr"));
  ASSERT_EQ("i", parser.getString("stdin"));
  ASSERT_EQ("o", parser.getString("stdout"));
  ASSERT_EQ("wd", parser.getString("work-dir"));

  ASSERT_EQ("rt", parser.getString("repo-type"));
  ASSERT_EQ("rp", parser.getString("repo-path"));
  ASSERT_EQ("rv", parser.getString("repo-version"));

  ASSERT_EQ("ms",  parser.getString("manager-socket"));
  ASSERT_EQ("ss",  parser.getString("spawner-socket"));
  ASSERT_EQ("sms", parser.getString("spawner-manager-socket"));
}
