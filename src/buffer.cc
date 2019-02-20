#include "buffer.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

namespace hera {

Buffer::Buffer(size_t capacity) : capacity_(capacity), position_(0),
    limit_(0) {
  buf_ = reinterpret_cast<char*>(malloc(capacity_));
  assert(buf_ != nullptr);
}

Buffer::~Buffer() {
  free(buf_);
}

void Buffer::Write(const void* buf, size_t size) {
  Expand(size);
  memcpy(buf_ + limit_, buf, size);
  limit_ += size;
}

void Buffer::Expand(size_t size) {
  size_t new_size = capacity_;
  while (new_size - limit_ < size) {
    new_size <<= 1;
  }
  if (new_size == capacity_) {
    return;
  }
  buf_ = reinterpret_cast<char*>(realloc(buf_, new_size));
  assert(buf_ != nullptr);
  capacity_ = new_size;
}

void Buffer::Clear() {
  position_ = 0;
  limit_ = 0;
}

}  // namespace hera
