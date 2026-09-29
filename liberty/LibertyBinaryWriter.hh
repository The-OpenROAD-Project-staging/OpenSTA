// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025-2026, The OpenROAD Authors

#pragma once

#include <cstdint>
#include <iosfwd>
#include <unordered_map>
#include "LibertyBinaryCommon.hh"
#include "LibertyParser.hh"

namespace sta {

// Heterogeneous lookup so the table can be probed with a string_view
// without allocating a key.
struct StringViewHash {
  using is_transparent = void;
  size_t operator()(std::string_view str) const {
    return std::hash<std::string_view>()(str);
  }
};
using LibertyStringTable =
  std::unordered_map<std::string, std::uint32_t, StringViewHash, std::equal_to<>>;

class LibertyBinaryWriter : public LibertyGroupVisitor
{
public:
  LibertyBinaryWriter(std::ostream *stream);
  virtual ~LibertyBinaryWriter() = default;

  virtual void begin(const LibertyGroup *group,
                     LibertyGroup *parent_group) override;
  virtual void end(const LibertyGroup *group,
                   LibertyGroup *parent_group) override;
  virtual void visitAttr(const LibertySimpleAttr *attr) override;
  virtual void visitAttr(const LibertyComplexAttr *attr) override;
  virtual void visitVariable(LibertyVariable *variable) override;
  const LibertyStringTable &string_table() const { return string_table_; }

private:
  void writeTag(LibertyBinaryTag tag);
  void writeType(LibertyBinaryValueType type);
  void writeString(std::string_view str);
  void writeFloat(float val);
  void writeValue(const LibertyAttrValue *value);
  void writeFloatSeq(const std::vector<float> &floats);
  // Source line of a statement, as a zigzag varint delta from the previous
  // statement's line.
  void writeLine(int line);
  void writeVarint(std::uint32_t val);

  std::ostream *stream_;
  LibertyStringTable string_table_;
  // Open group nesting depth; used to free top-level group subtrees once
  // serialized so large libraries don't accumulate in memory.
  int depth_ = 0;
  // Line of the previous statement, for delta encoding.
  int last_line_ = 0;
};

void
writeLibertyBinary(const char *in_filename,
                   const char *out_filename,
                   Report *report);

} // namespace
