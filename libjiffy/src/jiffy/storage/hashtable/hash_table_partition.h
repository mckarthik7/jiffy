#ifndef JIFFY_KV_SERVICE_SHARD_H
#define JIFFY_KV_SERVICE_SHARD_H

#include <libcuckoo/cuckoohash_map.hh>

#include <string>
#include "serde/serde.h"
#include "serde/binary_serde.h"
#include "jiffy/storage/partition.h"
#include "../../persistent/persistent_service.h"
#include "../chain_module.h"
#include "hash_table_defs.h"
#include "serde/csv_serde.h"

namespace jiffy {
namespace storage {

typedef std::string binary; // Since thrift translates binary to string
typedef binary key_type;
typedef binary value_type;

/**
 * Hash partition state
 */

enum hash_partition_state {
  regular = 0,
  importing = 1,
  exporting = 2
};

extern std::vector<command> KV_OPS;
/**
 * @brief Key value block supported operations
 */
enum kv_op_id : int32_t {
  exists = 0,
  get = 1,
  keys = 2, // TODO: We should not support multi-key operations since we do not provide any guarantees
  num_keys = 3, // TODO: We should not support multi-key operations since we do not provide any guarantees
  put = 4,
  remove = 5,
  update = 6,
  lock = 7,
  unlock = 8,
  locked_data_in_slot_range = 9,
  locked_get = 10,
  locked_put = 11,
  locked_remove = 12,
  locked_update = 13,
  upsert = 14,
  locked_upsert = 15
};

/* Key value partition structure class, inherited from chain module */
class hash_table_partition : public chain_module {
 public:
  /* Slot range max */
  static const int32_t SLOT_MAX = 65536;

  /**
   * @brief Constructor
   * @param block_name Block name
   * @param capacity Block capacity
   * @param threshold_lo low threshold
   * @param threshold_hi high threshold
   * @param directory_host Directory hostname
   * @param directory_port Directory port number
   * @param ser Custom serializer/deserializer
   */

  explicit hash_table_partition(const std::string &block_name,
                                std::size_t capacity = 134217728, // 128 MB; TODO: hardcoded default
                                double threshold_lo = 0.05,
                                double threshold_hi = 0.95,
                                const std::string &directory_host = "127.0.0.1",
                                int directory_port = 9090,
                                std::shared_ptr<serde> ser = std::make_shared<csv_serde>());

  virtual ~hash_table_partition() = default;

  void setup(const std::string &path,
             const std::string &partition_name,
             const std::string &partition_metadata,
             const std::vector<std::string> &chain,
             chain_role role,
             const std::string &next_block_name) override;

  /**
   * @brief Set block hash slot range
   * @param slot_begin Begin slot
   * @param slot_end End slot
   */

  void slot_range(int32_t slot_begin, int32_t slot_end) {
    std::unique_lock<std::shared_mutex> lock(metadata_mtx_);
    slot_range_.first = slot_begin;
    slot_range_.second = slot_end;
  }

  /**
   * @brief Fetch slot range
   * @return Block slot range
   */

  const std::pair<int32_t, int32_t> &slot_range() const {
    std::shared_lock<std::shared_mutex> lock(metadata_mtx_);
    return slot_range_;
  }

  /**
   * @brief Fetch begin slot
   * @return Begin slot
   */

  int32_t slot_begin() const {
    std::shared_lock<std::shared_mutex> lock(metadata_mtx_);
    return slot_range_.first;
  }

  /**
   * @brief Fetch end slot
   * @return End slot
   */

  int32_t slot_end() const {
    std::shared_lock<std::shared_mutex> lock(metadata_mtx_);
    return slot_range_.second;
  }

  /**
   * @brief Check if slot is within the slot range
   * @param slot Slot
   * @return Bool value, true if slot is within the range
   */

  bool in_slot_range(int32_t slot) {
    std::shared_lock<std::shared_mutex> lock(metadata_mtx_);
    return slot >= slot_range_.first && slot <= slot_range_.second;
  }

  /**
   * @brief Set block state
   * @param state Block state
   */

  void state(hash_partition_state state) {
    std::unique_lock<std::shared_mutex> lock(metadata_mtx_);
    state_ = state;
  }

  /**
   * @brief Fetch block state
   * @return Block state
   */

  const hash_partition_state &state() const {
    std::shared_lock<std::shared_mutex> lock(metadata_mtx_);
    return state_;
  }

  /**
   * @brief Set export slot range
   * @param slot_begin Begin slot
   * @param slot_end End slot
   */

  void export_slot_range(int32_t slot_begin, int32_t slot_end) {
    std::unique_lock<std::shared_mutex> lock(metadata_mtx_);
    export_slot_range_.first = slot_begin;
    export_slot_range_.second = slot_end;
  }

  /**
   * @brief Fetch export slot range
   * @return Export slot range
   */

  const std::pair<int32_t, int32_t> &export_slot_range() {
    std::shared_lock<std::shared_mutex> lock(metadata_mtx_);
    return export_slot_range_;
  };

  /**
   * @brief Check if slot is within export slot range
   * @param slot Slot
   * @return Bool value, true if slot is within the range
   */

  bool in_export_slot_range(int32_t slot) {
    std::shared_lock<std::shared_mutex> lock(metadata_mtx_);
    return slot >= export_slot_range_.first && slot <= export_slot_range_.second;
  }

  /**
   * @brief Set import slot range
   * @param slot_begin Begin slot
   * @param slot_end End slot
   */

  void import_slot_range(int32_t slot_begin, int32_t slot_end) {
    std::unique_lock<std::shared_mutex> lock(metadata_mtx_);
    import_slot_range_.first = slot_begin;
    import_slot_range_.second = slot_end;
  }

  /**
   * @brief Fetch import slot range
   * @return Import slot range
   */

  const std::pair<int32_t, int32_t> &import_slot_range() {
    std::shared_lock<std::shared_mutex> lock(metadata_mtx_);
    return import_slot_range_;
  };

  /**
   * @brief Check if slot is within import slot range
   * @param slot Slot
   * @return Bool value, true if slot is within the range
   */

  bool in_import_slot_range(int32_t slot) {
    return slot >= import_slot_range_.first && slot <= import_slot_range_.second;
  }

  /**
   * @brief Set the export target
   * @param target Export target
   */

  void export_target(const std::vector<std::string> &target) {
    std::unique_lock<std::shared_mutex> lock(metadata_mtx_);
    export_target_ = target;
    export_target_str_ = "";
    for (const auto &block: target) {
      export_target_str_ += (block + "!");
    }
    export_target_str_.pop_back();
  }

  /**
   * @brief Fetch export target
   * @return Export target
   */

  const std::vector<std::string> &export_target() const {
    std::shared_lock<std::shared_mutex> lock(metadata_mtx_);
    return export_target_;
  }

  /**
   * @brief Fetch export target string
   * @return Export target string
   */

  const std::string export_target_str() const {
    std::shared_lock<std::shared_mutex> lock(metadata_mtx_);
    return export_target_str_;
  }

  /**
   * @brief Check if hash map contains key
   * @param key Key
   * @param redirect Bool value to choose whether to indirect to the destination
   * block when block is in repartitioning
   * @return String of key status
   */

  std::string exists(const key_type &key, bool redirect = false);

  /**
   * @brief Put new key value pair
   * @param key Key
   * @param value Value
   * @param redirect Bool value to choose whether to indirect to the destination
   * block when block is in repartitioning
   * @return Put status string
   */

  std::string put(const key_type &key, const value_type &value, bool redirect = false);

  /**
   * @brief Put new key value pair in locked block
   * @param key Key
   * @param value Value
   * @param redirect Bool value to choose whether to indirect to the destination
   * block when block is in repartitioning
   * @return Put status string
   */

  std::string locked_put(const key_type &key, const value_type &value, bool redirect = false);

  /**
   * @brief Insert with the maximum value for specified key
   * @param key Key
   * @param value Value
   * @param redirect Bool value to choose whether to indirect to the destination
   * block when block is in repartitioning
   * @return Upsert status string
   */

  std::string upsert(const key_type &key, const value_type &value, bool redirect = false);

  /**
   * @brief Insert with the maximum value for specified key in locked block
   * @param key Key
   * @param value Value
   * @param redirect Bool value to choose whether to indirect to the destination
   * block when block is in repartitioning
   * @return Upsert status string
   */

  std::string locked_upsert(const key_type &key, const value_type &value, bool redirect = false);

  /**
   * @brief Get value for specified key
   * @param key Key
   * @param redirect Bool value to choose whether to indirect to the destination
   * block when block is in repartitioning
   * @return Get status string
   */

  value_type get(const key_type &key, bool redirect = false);

  /**
   * @brief Get value for specified key in locked block
   * @param key Key
   * @param redirect Bool value to choose whether to indirect to the destination
   * block when block is in repartitioning
   * @return Get status string
   */

  std::string locked_get(const key_type &key, bool redirect = false);

  /**
   * @brief Update the value for specified key
   * @param key Key
   * @param value Value
   * @param redirect Bool value to choose whether to indirect to the destination
   * block when block is in repartitioning
   * @return Update status string
   */

  std::string update(const key_type &key, const value_type &value, bool redirect = false);

  /**
   * @brief Update the value for specified key in locked block
   * @param key Key
   * @param value Value
   * @param redirect Bool value to choose whether to indirect to the destination
   * block when block is in repartitioning
   * @return Update status string
   */

  std::string locked_update(const key_type &key, const value_type &value, bool redirect = false);

  /**
   * @brief Remove value for specified key
   * @param key Key
   * @param redirect Bool value to choose whether to indirect to the destination
   * block when block is in repartitioning
   * @return Remove status string
   */

  std::string remove(const key_type &key, bool redirect = false);

  /**
   * @brief Remove value for specified key in locked block
   * @param key Key
   * @param redirect Bool value to choose whether to indirect to the destination
   * block when block is in repartitioning
   * @return Remove status
   */

  std::string locked_remove(const key_type &key, bool redirect = false);

  /**
   * @brief Return keys
   * @param keys Keys
   * TODO to be removed?
   */

  void keys(std::vector<std::string> &keys);

  /**
   * @brief Fetch data from keys which lie in slot range
   * @param data Data to be fetched
   * @param slot_begin Slot begin value
   * @param slot_end Slot end value
   * @param num_keys Key numbers to be fetched
   */

  void locked_get_data_in_slot_range(std::vector<std::string> &data,
                                     int32_t slot_begin,
                                     int32_t slot_end,
                                     int32_t num_keys);

  /**
   * @brief Active lock block
   * @return Lock status string
   */

  std::string lock();

  /**
   * @brief Unlock the locked block
   * @return Unlock status string
   */

  std::string unlock();

  /**
   * @brief Check if key value block is locked
   * @return Bool value, true if locked
   */

  bool is_locked();

  /**
   * @brief Fetch block size
   * @return Block size
   */

  std::size_t size() const;

  /**
   * @brief Check if block is empty
   * @return Bool value, true if empty
   */

  bool empty() const;

  /**
   * @brief Run particular command on key value block
   * @param _return Return status to be collected
   * @param oid Operation identifier
   * @param args Command arguments
   */

  void run_command(std::vector<std::string> &_return, int oid, const std::vector<std::string> &args) override;

  /**
   * @brief Atomically check dirty bit
   * @return Bool value, true if block is dirty
   */

  bool is_dirty() const;

  /**
   * @brief Load persistent data into the block, lock the block while doing this
   * @param path Persistent storage path
   */

  void load(const std::string &path) override;

  /**
   * @brief If dirty, synchronize persistent storage and block
   * @param path Persistent storage path
   * @return Bool value, true if block successfully synchronized
   */

  bool sync(const std::string &path) override;

  /**
   * @brief Flush the block if dirty and clear the block
   * @param path Persistent storage path
   * @return Bool value, true if block successfully dumped
   */

  bool dump(const std::string &path) override;

  /**
   * @brief Fetch block storage capacity
   * @return Block storage capacity
   */

  std::size_t storage_capacity() override;

  /**
   * @brief Fetch block storage size, total size of all the keys and values
   * @return Block storage size
   */

  std::size_t storage_size() override;

  /**
   * @brief Reset the block
   */

  void reset() override;

  /**
   * @brief Send all key and value to the next block
   */

  void forward_all() override;

  /**
   * @brief Export slots
   */

  void export_slots() override;

  /**
   * @brief Set block to be exporting
   * @param target_block Export target block
   * @param slot_begin Begin slot
   * @param slot_end End slot
   */

  void set_exporting(const std::vector<std::string> &target_block, int32_t slot_begin, int32_t slot_end) {
    std::unique_lock<std::shared_mutex> lock(metadata_mtx_);
    state_ = hash_partition_state::exporting;
    export_target_ = target_block;
    export_target_str_ = "";
    for (const auto &block: target_block) {
      export_target_str_ += (block + "!");
    }
    export_target_str_.pop_back();
    export_slot_range_.first = slot_begin;
    export_slot_range_.second = slot_end;
  }

  /**
   * @brief Set block to be importing
   * @param slot_begin Begin slot
   * @param slot_end End slot
   */

  void set_importing(int32_t slot_begin, int32_t slot_end) {
    std::unique_lock<std::shared_mutex> lock(metadata_mtx_);
    state_ = hash_partition_state::importing;
    import_slot_range_.first = slot_begin;
    import_slot_range_.second = slot_end;
  }

  /**
   * @brief Set block to regular after exporting slot
   * @param slot_begin Begin slot
   * @param slot_end End slot
   */

  void set_regular(int32_t slot_begin, int32_t slot_end) {
    std::unique_lock<std::shared_mutex> lock(metadata_mtx_);
    state_ = hash_partition_state::regular;
    slot_range_.first = slot_begin;
    slot_range_.second = slot_end;
    export_slot_range_.first = 0;
    export_slot_range_.second = -1;
    import_slot_range_.first = 0;
    import_slot_range_.second = -1;
  }

 private:
  /**
   * @brief Check if block is overloaded
   * @return Bool value, true if block size is over the high threshold capacity
   */

  bool overload();

  /**
   * @brief Check if block is underloaded
   * @return Bool value, true if block size is under the low threshold capacity
   */

  bool underload();

  /* Cuckoo hash map partition */
  hash_table_type block_;

  /* Locked cuckoo hash map partition */
  locked_hash_table_type locked_block_;

  /* Directory host number */
  std::string directory_host_;

  /* Directory port number */
  int directory_port_;

  /* Custom serializer/deserializer */
  std::shared_ptr<serde> ser_;

  /* Atomic value to collect the sum of key size and value size */
  std::atomic<size_t> bytes_;

  /* Key value partition capacity */
  std::size_t capacity_;

  /**
   * @brief The two threshold to determine whether the block
   * is overloaded or underloaded
   */
  /* Low threshold */
  double threshold_lo_;
  /* High threshold */
  double threshold_hi_;

  /* Atomic bool for partition hash slot range splitting */
  std::atomic<bool> splitting_;

  /* Atomic bool for partition hash slot range merging */
  std::atomic<bool> merging_;

  /* Atomic partition dirty bit */
  std::atomic<bool> dirty_;

  /* Block state, regular, importing or exporting */
  hash_partition_state state_;
  /* Hash slot range */
  std::pair<int32_t, int32_t> slot_range_;
  /* Bool value for auto scaling */
  std::atomic_bool auto_scale_;
  /* Export slot range */
  std::pair<int32_t, int32_t> export_slot_range_;
  /* Export targets */
  std::vector<std::string> export_target_;
  /* String representation for export target */
  std::string export_target_str_;
  /* Import slot range */
  std::pair<int32_t, int32_t> import_slot_range_;
};

}
}

#endif //JIFFY_KV_SERVICE_SHARD_H