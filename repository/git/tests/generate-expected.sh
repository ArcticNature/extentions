#!/bin/bash
#  Generates the variables and code for the current state of the repo.


# Functions.
get_id() {
  type=$1
  object=$2
  git show --no-patch --pretty=format:${type} "${object}"
}
tree_id() {
  get_id "%T" "$1"
}
commit_id() {
  get_id "%H" "$1"
}


# Needed values.
BRANCH_NAME="ext.repo.git.fixture"
BRANCH_TREE=$(tree_id "${BRANCH_NAME}")
BRANCH_COMMIT=$(commit_id "${BRANCH_NAME}")
TAG_NAME="v0.0.3"
TAG_COMMIT=$(commit_id "${TAG_NAME}")


# Print template.
echo -e "
const std::string BRANCH_NAME   = \"${BRANCH_NAME}\";
const std::string BRANCH_TREE   = \"${BRANCH_TREE}\";
const std::string BRANCH_COMMIT = \"${BRANCH_COMMIT}\";
const std::string TAG_NAME      = \"${TAG_NAME}\";
const std::string TAG_COMMIT    = \"${TAG_COMMIT}\";
"
