#include <vector>
#include <thread>
#include <boost/program_options.hpp>
#include <jiffy/client/jiffy_client.h>
#include <jiffy/utils/logger.h>
#include <jiffy/utils/signal_handling.h>
#include <jiffy/utils/time_utils.h>
#include <fstream>
#include <atomic>
#include <chrono>

using namespace ::jiffy::client;
using namespace ::jiffy::directory;
using namespace ::jiffy::storage;
using namespace ::jiffy::utils;
using namespace ::apache::thrift;
namespace ts = std::chrono;
int main() {
  std::string address = "127.0.0.1";
  int service_port = 9090;
  int lease_port = 9091;
  int num_blocks = 1;
  int chain_length = 1;
  size_t num_ops = 419430;
  size_t data_size = 102400;
  std::string op_type = "file_auto_scaling";
  std::string path = "/tmp";
  std::string backing_path = "local://tmp";

  // Output all the configuration parameters:
  LOG(log_level::info) << "host: " << address;
  LOG(log_level::info) << "service-port: " << service_port;
  LOG(log_level::info) << "lease-port: " << lease_port;
  LOG(log_level::info) << "num-blocks: " << num_blocks;
  LOG(log_level::info) << "chain-length: " << chain_length;
  LOG(log_level::info) << "num-ops: " << num_ops;
  LOG(log_level::info) << "data-size: " << data_size;
  LOG(log_level::info) << "test: " << op_type;
  LOG(log_level::info) << "path: " << path;
  LOG(log_level::info) << "backing-path: " << backing_path;

  jiffy_client client(address, service_port, lease_port);
  std::shared_ptr<file_client>
      file_client_1 = client.open_or_create_file(path, backing_path, num_blocks, chain_length);
  std::string data_(data_size, 'x');
  std::chrono::milliseconds periodicity_ms_(1000);
  std::atomic_bool stop_{false};
  std::atomic_bool stop2_{false};
  std::size_t j = 0;
  auto worker_ = std::thread([&] {
    std::ofstream out("dataset.trace");
    while (!stop_.load()) {
      auto start = std::chrono::steady_clock::now();
      try {
        auto cur_epoch = ts::duration_cast<ts::milliseconds>(ts::system_clock::now().time_since_epoch()).count();
        out << cur_epoch;
        out << "\t" << j * 100 * 1024;
        out << std::endl;
      } catch (std::exception &e) {
        LOG(log_level::error) << "Exception: " << e.what();
      }
      auto end = std::chrono::steady_clock::now();
      auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

      auto time_to_wait = std::chrono::duration_cast<std::chrono::milliseconds>(periodicity_ms_ - elapsed);
      if (time_to_wait > std::chrono::milliseconds::zero()) {
        std::this_thread::sleep_for(time_to_wait);
      }
    }
    out.close();
  });
  auto read_worker_ = std::thread([&] {
    uint64_t read_tot_time = 0, read_t0 = 0, read_t1 = 0;
    std::shared_ptr<file_client>
        file_client_2 = client.open_file(path);
    std::ofstream out2("read_latency.trace");
    while (!stop2_.load()) {
      for (size_t k = 0; k < num_ops; ++k) {
        read_t0 = time_utils::now_us();
        file_client_2->read(data_size);
        read_t1 = time_utils::now_us();
        read_tot_time = (read_t1 - read_t0);
        auto cur_epoch = ts::duration_cast<ts::milliseconds>(ts::system_clock::now().time_since_epoch()).count();
        out2 << cur_epoch << " " << read_tot_time << " read" << std::endl;
      }
    }
    out2.close();
  });

  std::ofstream out("latency.trace");
  uint64_t write_tot_time = 0, write_t0 = 0, write_t1 = 0;
  for (j = 0; j < num_ops; ++j) {
    write_t0 = time_utils::now_us();
    file_client_1->write(data_);
    write_t1 = time_utils::now_us();
    write_tot_time = write_t1 - write_t0;
    auto cur_epoch = ts::duration_cast<ts::milliseconds>(ts::system_clock::now().time_since_epoch()).count();
    out << cur_epoch << " " << write_tot_time << " us";
    out << std::endl;
  }
  stop_.store(true);
  if (worker_.joinable())
    worker_.join();
  stop2_.store(true);
  if (read_worker_.joinable())
    read_worker_.join();
  client.remove(path);
  return 0;
}