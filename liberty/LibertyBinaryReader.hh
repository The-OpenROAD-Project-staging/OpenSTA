// OpenSTA, Static Timing Analyzer
// Copyright (c) 2025, Parallax Software, Inc.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <iosfwd>
#include <string>
#include <string_view>
#include <vector>
#include <cstdint>
#include <cstring>
#include "LibertyParser.hh"

namespace sta {

// Unchecked read primitives; LibertyBinaryReader bounds-checks with
// remaining()/inBounds() before every read so a malformed or foreign-format
// file is rejected rather than read out of bounds.
class BinaryCursor
{
public:
  BinaryCursor(const char *data,
               size_t size) :
    ptr_(data),
    end_(data + size),
    start_(data)
  {
  }

  std::uint8_t readU8() { return static_cast<std::uint8_t>(*ptr_++); }
  // Multi-byte values are in the host's byte order (LibertyBinaryCommon.hh
  // asserts it is little-endian), so the memcpy is a plain load.
  std::uint32_t readU32()
  {
    std::uint32_t val;
    std::memcpy(&val, ptr_, sizeof(val));
    ptr_ += sizeof(val);
    return val;
  }
  std::uint64_t readU64()
  {
    std::uint64_t val;
    std::memcpy(&val, ptr_, sizeof(val));
    ptr_ += sizeof(val);
    return val;
  }
  float readFloat()
  {
    float val;
    std::memcpy(&val, ptr_, sizeof(val));
    ptr_ += sizeof(val);
    return val;
  }
  void readBytes(char *dest,
                 size_t len)
  {
    std::memcpy(dest, ptr_, len);
    ptr_ += len;
  }

  void seek(size_t offset) { ptr_ = start_ + offset; }
  void setPtr(const char *ptr) { ptr_ = ptr; }
  const char *current() const { return ptr_; }
  size_t remaining() const { return ptr_ < end_ ? size_t(end_ - ptr_) : 0; }
  bool inBounds(size_t offset) const { return offset <= size_t(end_ - start_); }

private:
  const char *ptr_;
  const char *end_;
  const char *start_;
};

class LibertyBinaryReader
{
public:
  LibertyBinaryReader(LibertyGroupVisitor *visitor,
                      std::string_view filename,
                      Report *report);
  ~LibertyBinaryReader() = default;

  // Errors on a malformed file rather than returning.
  void read(std::istream *stream);

private:
  // Walk the binary stream, driving the parser's builder methods so the
  // group/attribute ownership and visitor dispatch match the text reader.
  // Only groups are legal at the top level.
  void readStatements(bool top_level);
  void readGroup();
  void readSimpleAttr();
  void readComplexAttr();
  void readVariable();
  // Source line of the statement being read (zigzag varint delta).
  int readLine();
  std::uint32_t readVarint();

  // Helpers
  void readStringTable();
  // Type-tagged reads for structural fields.
  std::string readString();
  float readFloat();
  std::uint32_t readUInt32();
  // Payload reads; readValue has already consumed the type tag.
  std::string readStringIndex();
  float readFloatValue();
  LibertyAttrValue *readValue();
  // Counted value sequence; returns nullptr instead of allocating when empty.
  LibertyAttrValueSeq *readValues();
  // Error unless bytes remain before the end of the buffer.
  void require(size_t bytes);
  // Report::error throws, so these do not return.
  void corruptError();
  void versionError(std::uint32_t version);

  LibertyParser parser_;
  BinaryCursor cursor_;
  std::vector<std::string> string_table_;
  // Line of the previous statement; statement lines are stored as deltas.
  int last_line_ = 0;
};
} // namespace
