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

#ifndef SRC_TINT_WRITER_SPIRV_GENERATOR_IMPL_IR_H_
#define SRC_TINT_WRITER_SPIRV_GENERATOR_IMPL_IR_H_

#include <vector>

#include "src/tint/diagnostic/diagnostic.h"
#include "src/tint/utils/hashmap.h"
#include "src/tint/utils/vector.h"
#include "src/tint/writer/spirv/binary_writer.h"
#include "src/tint/writer/spirv/module.h"

// Forward declarations
namespace tint::ir {
class Function;
class Module;
}  // namespace tint::ir
namespace tint::type {
class Type;
}  // namespace tint::type

namespace tint::writer::spirv {

/// Implementation class for SPIR-V generator
class GeneratorImplIr {
  public:
    /// Constructor
    /// @param module the Tint IR module to generate
    /// @param zero_init_workgroup_memory `true` to initialize all the variables in the Workgroup
    ///                                   storage class with OpConstantNull
    GeneratorImplIr(const ir::Module* module, bool zero_init_workgroup_memory);

    /// @returns true on successful generation; false otherwise
    bool Generate();

    /// @returns the module that this generator has produced
    spirv::Module& Module() { return module_; }

    /// @returns the generated SPIR-V binary data
    const std::vector<uint32_t>& Result() const { return writer_.result(); }

    /// @returns the list of diagnostics raised by the generator
    diag::List Diagnostics() const { return diagnostics_; }

    /// Get the result ID of the type `ty`, emitting a type declaration instruction if necessary.
    /// @param ty the type to get the ID for
    /// @returns the result ID of the type
    uint32_t Type(const type::Type* ty);

    /// Emit a function.
    /// @param func the function to emit
    void EmitFunction(const ir::Function* func);

  private:
    const ir::Module* ir_;
    spirv::Module module_;
    BinaryWriter writer_;
    diag::List diagnostics_;

    /// A function type used for an OpTypeFunction declaration.
    struct FunctionType {
        uint32_t return_type_id;
        utils::Vector<uint32_t, 4> param_type_ids;

        /// Hasher provides a hash function for the FunctionType.
        struct Hasher {
            /// @param ft the FunctionType to create a hash for
            /// @return the hash value
            inline std::size_t operator()(const FunctionType& ft) const {
                size_t hash = utils::Hash(ft.return_type_id);
                for (auto& p : ft.param_type_ids) {
                    hash = utils::HashCombine(hash, p);
                }
                return hash;
            }
        };

        /// Equality operator for FunctionType.
        bool operator==(const FunctionType& other) const {
            return (param_type_ids == other.param_type_ids) &&
                   (return_type_id == other.return_type_id);
        }
    };

    /// The map of types to their result IDs.
    utils::Hashmap<const type::Type*, uint32_t, 8> types_;

    /// The map of function types to their result IDs.
    utils::Hashmap<FunctionType, uint32_t, 8, FunctionType::Hasher> function_types_;

    bool zero_init_workgroup_memory_ = false;
};

}  // namespace tint::writer::spirv

#endif  // SRC_TINT_WRITER_SPIRV_GENERATOR_IMPL_IR_H_
