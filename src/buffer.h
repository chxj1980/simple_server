#pragma once

#include <stddef.h>

#include <memory>
#include <vector>

namespace hera {

class Buffer {
 public:
  explicit Buffer(size_t capacity);
  ~Buffer();

  Buffer(const Buffer&) = delete;
  Buffer(Buffer&&) = delete;
  Buffer& operator=(const Buffer&) = delete;
  Buffer& operator=(Buffer&&) = delete;

  void Write(const void* buf, size_t count);
  void Expand(size_t size);
  void Clear();

  const char* head() const { return buf_ + position_; }
  char* tail() const { return buf_ + limit_; }
  size_t capacity() const { return capacity_; }
  size_t position() const { return position_; }
  void incr_position(size_t size) { position_ += size; }
  size_t limit() const { return limit_; }
  void incr_limit(size_t size) { limit_ += size; }
  size_t remaining() const { return capacity_ - limit_; }
  size_t size() const { return limit_ - position_; }
  bool empty() const { return position_ == limit_; }

 private:
  char* buf_;
  size_t capacity_;
  size_t position_;
  size_t limit_;
};

using BufferPtr = std::shared_ptr<Buffer>;

}  // namespace hera
