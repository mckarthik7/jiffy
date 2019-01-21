#include <fstream>
#include <catch.hpp>
#include <iostream>
#include "jiffy/persistent/local/local_store.h"
#include "jiffy/utils/directory_utils.h"
#include "jiffy/storage/kv/serde/csv_serde.h"

using namespace ::jiffy::persistent;
using namespace ::jiffy::storage;

TEST_CASE("local_write_test", "[write]") {
  hash_table_type table;
  auto ltable = table.lock_table();
  ltable.insert("key", "value");

  auto ser = std::make_shared<csv_serde>();
  local_store store(ser);
  REQUIRE_NOTHROW(store.write(ltable, "/tmp/a.txt"));
  ltable.unlock();
  std::ifstream in("/tmp/a.txt", std::ifstream::in);
  std::string data;
  in >> data;
  REQUIRE(data == "key,value");
  std::remove("/tmp/a.txt");
}

TEST_CASE("local_read_test", "[read]") {
  std::ofstream out("/tmp/a.txt", std::ofstream::out);
  out << "key,value\n";
  out.close();

  auto ser = std::make_shared<csv_serde>();
  local_store store(ser);
  hash_table_type table;
  auto ltable = table.lock_table();
  REQUIRE_NOTHROW(store.read("/tmp/a.txt", ltable));
  REQUIRE(ltable.at("key") == "value");
  ltable.unlock();
  std::remove("/tmp/a.txt");
}