// Copyright 2020 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "src/reader/spirv/namer.h"

#include <algorithm>
#include <sstream>
#include <unordered_set>

namespace tint {
namespace reader {
namespace spirv {

namespace {

const char* kWGSLReservedWords[] = {
    // Please keep this list sorted
    "array",
    "as",
    "asm",
    "bf16",
    "binding",
    "block",
    "bool",
    "break",
    "builtin",
    "case",
    "cast",
    "compute",
    "const",
    "constant_id",
    "continue",
    "default",
    "discard",
    "do",
    "else",
    "elseif",
    "entry_point",
    "enum",
    "f16",
    "f32",
    "fallthrough",
    "false",
    "fn",
    "for",
    "fragment",
    "i16",
    "i32",
    "i64",
    "i8",
    "if",
    "image",
    "import",
    "in",
    "let",
    "location",
    "loop",
    "mat2x2",
    "mat2x3",
    "mat2x4",
    "mat3x2",
    "mat3x3",
    "mat3x4",
    "mat4x2",
    "mat4x3",
    "mat4x4",
    "offset",
    "out",
    "premerge",
    "private",
    "ptr",
    "regardless",
    "return",
    "set",
    "storage_buffer",
    "struct",
    "switch",
    "true",
    "type",
    "typedef",
    "u16",
    "u32",
    "u64",
    "u8",
    "uniform",
    "uniform_constant",
    "unless",
    "using",
    "var",
    "vec2",
    "vec3",
    "vec4",
    "vertex",
    "void",
    "while",
    "workgroup",
};

}  // namespace

Namer::Namer(const FailStream& fail_stream) : fail_stream_(fail_stream) {
  for (const auto* reserved : kWGSLReservedWords) {
    name_to_id_[std::string(reserved)] = 0;
  }
}

Namer::~Namer() = default;

std::string Namer::Sanitize(const std::string& suggested_name) {
  if (suggested_name.empty()) {
    return "empty";
  }
  // Otherwise, replace invalid characters by '_'.
  std::string result;
  std::string invalid_as_first_char = "_0123456789";
  std::string valid =
      "abcdefghijklmnopqrstuvwxyz"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "_0123456789";
  // If the first character is invalid for starting a WGSL identifier, then
  // prefix the result with "x".
  if ((std::string::npos != invalid_as_first_char.find(suggested_name[0])) ||
      (std::string::npos == valid.find(suggested_name[0]))) {
    result = "x";
  }
  std::transform(suggested_name.begin(), suggested_name.end(),
                 std::back_inserter(result), [&valid](const char c) {
                   return (std::string::npos == valid.find(c)) ? '_' : c;
                 });
  return result;
}

std::string Namer::GetMemberName(uint32_t struct_id,
                                 uint32_t member_index) const {
  std::string result;
  auto where = struct_member_names_.find(struct_id);
  if (where != struct_member_names_.end()) {
    auto& member_names = where->second;
    if (member_index < member_names.size()) {
      result = member_names[member_index];
    }
  }
  return result;
}

std::string Namer::FindUnusedDerivedName(const std::string& base_name) const {
  // Ensure uniqueness among names.
  std::string derived_name;
  for (int i = 0;; i++) {
    std::stringstream new_name_stream;
    new_name_stream << base_name;
    if (i > 0) {
      new_name_stream << "_" << i;
    }
    derived_name = new_name_stream.str();
    if (name_to_id_.count(derived_name) == 0) {
      break;
    }
  }
  return derived_name;
}

std::string Namer::MakeDerivedName(const std::string& base_name) {
  auto result = FindUnusedDerivedName(base_name);
  // Register it.
  name_to_id_[result] = 0;
  return result;
}

bool Namer::SaveName(uint32_t id, const std::string& name) {
  if (HasName(id)) {
    return Fail() << "internal error: ID " << id
                  << " already has registered name: " << id_to_name_[id];
  }
  id_to_name_[id] = name;
  name_to_id_[name] = id;
  return true;
}

bool Namer::SuggestSanitizedName(uint32_t id,
                                 const std::string& suggested_name) {
  if (HasName(id)) {
    return false;
  }

  return SaveName(id, FindUnusedDerivedName(Sanitize(suggested_name)));
}

bool Namer::SuggestSanitizedMemberName(uint32_t struct_id,
                                       uint32_t member_index,
                                       const std::string& suggested_name) {
  // Creates an empty vector the first time we visit this struct.
  auto& name_vector = struct_member_names_[struct_id];
  // Resizing will set new entries to the empty string.
  name_vector.resize(std::max(name_vector.size(), size_t(member_index + 1)));
  auto& entry = name_vector[member_index];
  if (entry.empty()) {
    entry = Sanitize(suggested_name);
    return true;
  }
  return false;
}

void Namer::ResolveMemberNamesForStruct(uint32_t struct_id,
                                        uint32_t num_members) {
  auto& name_vector = struct_member_names_[struct_id];
  // Resizing will set new entries to the empty string.
  // It would have been an error if the client had registered a name for
  // an out-of-bounds member index, so toss those away.
  name_vector.resize(num_members);

  std::unordered_set<std::string> used_names;

  // Returns a name, based on the suggestion, which does not equal
  // any name in the used_names set.
  auto disambiguate_name =
      [&used_names](const std::string& suggestion) -> std::string {
    if (used_names.find(suggestion) == used_names.end()) {
      // There is no collision.
      return suggestion;
    }

    uint32_t i = 1;
    std::string new_name;
    do {
      std::stringstream new_name_stream;
      new_name_stream << suggestion << "_" << i;
      new_name = new_name_stream.str();
      ++i;
    } while (used_names.find(new_name) != used_names.end());
    return new_name;
  };

  // First ensure uniqueness among names for which we have already taken
  // suggestions.
  for (auto& name : name_vector) {
    if (!name.empty()) {
      // This modifies the names in-place, i.e. update the name_vector
      // entries.
      name = disambiguate_name(name);
      used_names.insert(name);
    }
  }

  // Now ensure uniqueness among the rest.  Doing this in a second pass
  // allows us to preserve suggestions as much as possible.  Otherwise
  // a generated name such as 'field1' might collide with a user-suggested
  // name of 'field1' attached to a later member.
  uint32_t index = 0;
  for (auto& name : name_vector) {
    if (name.empty()) {
      std::stringstream suggestion;
      suggestion << "field" << index;
      // Again, modify the name-vector in-place.
      name = disambiguate_name(suggestion.str());
      used_names.insert(name);
    }
    index++;
  }
}

}  // namespace spirv
}  // namespace reader
}  // namespace tint
