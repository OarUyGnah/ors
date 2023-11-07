#include <utils/buffer.h>
using namespace ors::utils;

buffer::buffer() : buf(nullptr), len(0), del_fn(nullptr) {}

buffer::buffer(void *data, uint64_t length, deleter del_fn)
    : buf(data), len(length), del_fn(del_fn) {}

buffer::buffer(buffer &&b) : buf(b.buf), len(b.len), del_fn(b.del_fn) {
  b.buf = nullptr;
  b.len = 0;
  b.del_fn = nullptr;
}

buffer &buffer::operator=(buffer &&b) {
  if (del_fn)
    del_fn(buf);
  buf = b.buf;
  len = b.len;
  del_fn = b.del_fn;
  b.buf = nullptr;
  b.len = 0;
  b.del_fn = nullptr;
  return *this;
}

buffer::~buffer() {
  if (del_fn)
    del_fn(buf);
}

void *buffer::data() { return buf; }

const void *buffer::data() const { return buf; }

size_t buffer::length() { return len; }

size_t buffer::length() const { return len; }

void buffer::set_data(void *data, size_t length, deleter del_fn) {
  if (del_fn)
    del_fn(buf);
  buf = data;
  len = length;
  del_fn = del_fn;
}

void buffer::clear() {
  if (del_fn)
    del_fn(buf);
  buf = nullptr;
  len = 0;
  del_fn = nullptr;
}