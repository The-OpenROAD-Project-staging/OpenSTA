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

#include <bit>
#include <cstddef>
#include <cstdint>

namespace sta {

// Multi-byte values are written and read in the host's byte order.
static_assert(std::endian::native == std::endian::little,
              "binary liberty format assumes a little-endian host");

// Magic number for binary liberty files: "STALIB01"
inline constexpr char LIBERTY_BINARY_MAGIC[] = "STALIB01";
inline constexpr size_t LIBERTY_BINARY_MAGIC_SIZE = sizeof(LIBERTY_BINARY_MAGIC) - 1;
// File layout: magic, version, source path (u32 length + bytes), source hash
// (u64), string table offset (u64), records, string table.
// Bumped when the layout changes; the reader rejects other versions.
// 1: initial layout.
// 2: group, attribute and variable records carry the source line of the
//    statement after the tag, as a zigzag varint delta from the previous
//    statement's line.
// 3: the header records the absolute path of the source file and the FNV-1a
//    64-bit hash of its bytes, for traceability.
inline constexpr uint32_t LIBERTY_BINARY_VERSION = 3;

// FNV-1a 64-bit hash of the source file's bytes as stored on disk (compressed
// when it is gzipped), so a .blib can be traced to the exact file it came from.
inline constexpr uint64_t LIBERTY_BINARY_HASH_OFFSET = 14695981039346656037ull;
inline constexpr uint64_t LIBERTY_BINARY_HASH_PRIME = 1099511628211ull;

inline uint64_t
libertyBinaryHash(uint64_t hash,
                  const char *bytes,
                  size_t size)
{
  for (size_t i = 0; i < size; i++) {
    hash ^= static_cast<unsigned char>(bytes[i]);
    hash *= LIBERTY_BINARY_HASH_PRIME;
  }
  return hash;
}

// Zigzag mapping so the (rare) negative line delta also encodes compactly.
inline constexpr uint32_t
zigzagEncode(int32_t val)
{
  return (static_cast<uint32_t>(val) << 1) ^ static_cast<uint32_t>(val >> 31);
}

inline constexpr int32_t
zigzagDecode(uint32_t val)
{
  return static_cast<int32_t>(val >> 1) ^ -static_cast<int32_t>(val & 1);
}

enum class LibertyBinaryTag : uint8_t {
  GROUP_BEGIN = 1,
  GROUP_END = 2,
  ATTR_SIMPLE = 3,
  ATTR_COMPLEX = 4,
  DEFINE = 5, // Unlikely to be used if writing from semantic model, but good to have
  VARIABLE = 6,
  EOF_TAG = 0
};

enum class LibertyBinaryValueType : uint8_t {
  STRING = 1,
  FLOAT = 2,
  INT = 3,     // Reserved; no writer emits it.
  BOOLEAN = 4, // Reserved; no writer emits it.
  FLOAT_SEQ = 5
};

} // namespace sta
