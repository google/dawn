// Copyright 2023 The Tint Authors.
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

#ifndef SRC_TINT_READER_SPIRV_ATTRIBUTES_H_
#define SRC_TINT_READER_SPIRV_ATTRIBUTES_H_

#include "src/tint/ast/attribute.h"
#include "src/tint/builtin/builtin_value.h"
#include "src/tint/program_builder.h"
#include "src/tint/utils/enum_set.h"
#include "src/tint/utils/vector.h"

namespace tint::reader::spirv {

/// Attributes holds a vector of ast::Attribute pointers, and a enum-set of flags used to hold
/// additional metadata.
struct Attributes {
    /// Flags used by #flags.
    enum class Flags {
        kHasBuiltinSampleMask,
    };

    /// Adds the attributes and flags of @p other to this.
    /// @param other the other Attributes to combine into this
    void Add(const Attributes& other) {
        for (auto* attr : other.list) {
            list.Push(attr);
        }
        for (auto flag : other.flags) {
            flags.Add(flag);
        }
    }

    /// Adds the attribute @p attr to the list of attributes
    /// @param attr the attribute to add to this
    void Add(const ast::Attribute* attr) { list.Push(attr); }

    /// Adds the builtin to the attribute list, also marking any necessary flags
    /// @param builder the program builder
    /// @param source the source of the builtin attribute
    /// @param builtin the builtin attribute to add
    void Add(ProgramBuilder& builder, const Source& source, builtin::BuiltinValue builtin) {
        Add(builder.Builtin(source, builtin));
        if (builtin == builtin::BuiltinValue::kSampleMask) {
            flags.Add(Flags::kHasBuiltinSampleMask);
        }
    }

    /// @returns true if the attribute list contains an attribute with the type `T`.
    template <typename T>
    bool Has() const {
        return ast::HasAttribute<T>(list);
    }

    /// @returns the attribute with type `T` in the list, or nullptr if no attribute of the given
    /// type exists in list.
    template <typename T>
    const T* Get() const {
        return ast::GetAttribute<T>(list);
    }

    /// The attributes
    utils::Vector<const ast::Attribute*, 8> list;
    /// The additional metadata flags
    utils::EnumSet<Flags> flags;
};

}  // namespace tint::reader::spirv

#endif  // SRC_TINT_READER_SPIRV_ATTRIBUTES_H_
