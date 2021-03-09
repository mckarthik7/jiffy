#include "persistent_service.h"

namespace jiffy {
namespace persistent {

using namespace utils;

local_store_impl::local_store_impl(std::shared_ptr<storage::serde> ser) : persistent_service(std::move(ser)) {}

std::string local_store_impl::URI() {
  return "local";
}

#ifdef S3_EXTERNAL

s3_store_impl::s3_store_impl(std::shared_ptr<storage::serde> ser) : persistent_service(std::move(ser)), options_{} {
  options_.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Warn;
  Aws::InitAPI(options_);
}

s3_store_impl::~s3_store_impl() {
  Aws::ShutdownAPI(options_);
}

std::pair<std::string, std::string> s3_store_impl::extract_path_elements(const std::string &s3_path) {
  utils::directory_utils::check_path(s3_path);

  auto bucket_end = std::find(s3_path.begin() + 1, s3_path.end(), '/');
  std::string bucket_name = std::string(s3_path.begin() + 1, bucket_end);
  std::string key = (bucket_end == s3_path.end()) ? std::string() : std::string(bucket_end + 1, s3_path.end());
  return std::make_pair(bucket_name, key);
}

std::string s3_store_impl::URI() {
  return "s3";
}

#endif

}
}
