// Copyright 2021 The Tint Authors.
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

#include "src/ast/struct_member.h"
#include "src/sem/struct.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::Struct);
TINT_INSTANTIATE_TYPEINFO(tint::sem::StructMember);

namespace tint {
namespace sem {

Struct::Struct(type::Struct* type,
               StructMemberList members,
               uint32_t align,
               uint32_t size,
               uint32_t size_no_padding,
               std::unordered_set<ast::StorageClass> storage_class_usage,
               std::unordered_set<PipelineStageUsage> pipeline_stage_uses)
    : type_(type),
      members_(std::move(members)),
      align_(align),
      size_(size),
      size_no_padding_(size_no_padding),
      storage_class_usage_(std::move(storage_class_usage)),
      pipeline_stage_uses_(std::move(pipeline_stage_uses)) {}

Struct::~Struct() = default;

const StructMember* Struct::FindMember(Symbol name) const {
  for (auto* member : members_) {
    if (member->Declaration()->symbol() == name) {
      return member;
    }
  }
  return nullptr;
}

StructMember::StructMember(ast::StructMember* declaration,
                           uint32_t offset,
                           uint32_t align,
                           uint32_t size)
    : declaration_(declaration), offset_(offset), align_(align), size_(size) {}

StructMember::~StructMember() = default;

}  // namespace sem
}  // namespace tint
