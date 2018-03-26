#include "catch.hpp"

#include "test_utils.h"
#include "../src/directory/lease/lease_expiry_worker.h"
#include "../src/directory/block/random_block_allocator.h"

#define LEASE_PERIOD_MS 100
#define GRACE_PERIOD_MS 100

using namespace ::elasticmem::directory;

TEST_CASE("lease_manager_test") {
  using namespace std::chrono_literals;

  auto alloc = std::make_shared<dummy_block_allocator>(4);
  auto sm = std::make_shared<dummy_storage_manager>();
  auto tree = std::make_shared<directory_tree>(alloc, sm);
  lease_expiry_worker mgr(tree, LEASE_PERIOD_MS, GRACE_PERIOD_MS);
  REQUIRE_NOTHROW(tree->create("/sandbox/a/b/c/file.txt", "/tmp", 1, 1));
  REQUIRE_NOTHROW(tree->create("/sandbox/a/b/file.txt", "/tmp", 1, 1));
  REQUIRE_NOTHROW(tree->create("/sandbox/a/file.txt", "/tmp", 1, 1));
  REQUIRE(tree->exists("/sandbox/a/b/c/file.txt"));
  REQUIRE(tree->exists("/sandbox/a/b/file.txt"));
  REQUIRE(tree->exists("/sandbox/a/file.txt"));

  REQUIRE_NOTHROW(mgr.start());
  std::this_thread::sleep_for(100ms);
  REQUIRE_NOTHROW(tree->touch("/sandbox/a/b/c"));
  std::this_thread::sleep_for(150ms);
  REQUIRE(tree->exists("/sandbox/a/b/c/file.txt"));
  REQUIRE(!tree->exists("/sandbox/a/b/file.txt"));
  REQUIRE(!tree->exists("/sandbox/a/file.txt"));
  REQUIRE(sm->COMMANDS.size() == 5);
  REQUIRE(sm->COMMANDS[0] == "setup_block:0:/sandbox/a/b/c/file.txt:0:nil");
  REQUIRE(sm->COMMANDS[1] == "setup_block:1:/sandbox/a/b/file.txt:0:nil");
  REQUIRE(sm->COMMANDS[2] == "setup_block:2:/sandbox/a/file.txt:0:nil");
  REQUIRE(sm->COMMANDS[3] == "reset:1");
  REQUIRE(sm->COMMANDS[4] == "reset:2");

  REQUIRE_NOTHROW(mgr.stop());
}