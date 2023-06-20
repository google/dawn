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

#ifndef SRC_TINT_WRITER_SPIRV_IR_GENERATOR_IMPL_IR_H_
#define SRC_TINT_WRITER_SPIRV_IR_GENERATOR_IMPL_IR_H_

#include <vector>

#include "src/tint/constant/value.h"
#include "src/tint/diagnostic/diagnostic.h"
#include "src/tint/ir/constant.h"
#include "src/tint/symbol.h"
#include "src/tint/utils/hashmap.h"
#include "src/tint/utils/vector.h"
#include "src/tint/writer/spirv/binary_writer.h"
#include "src/tint/writer/spirv/function.h"
#include "src/tint/writer/spirv/module.h"

// Forward declarations
namespace tint::ir {
class Access;
class Binary;
class Block;
class BlockParam;
class Branch;
class BuiltinCall;
class Construct;
class ControlInstruction;
class ExitIf;
class ExitLoop;
class ExitSwitch;
class Function;
class If;
class Load;
class Loop;
class Module;
class MultiInBlock;
class Store;
class Switch;
class UserCall;
class Value;
class Var;
}  // namespace tint::ir
namespace tint::type {
class Struct;
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
    GeneratorImplIr(ir::Module* module, bool zero_init_workgroup_memory);

    /// @returns true on successful generation; false otherwise
    bool Generate();

    /// @returns the module that this generator has produced
    spirv::Module& Module() { return module_; }

    /// @returns the generated SPIR-V binary data
    const std::vector<uint32_t>& Result() const { return writer_.result(); }

    /// @returns the list of diagnostics raised by the generator
    diag::List Diagnostics() const { return diagnostics_; }

    /// Get the result ID of the constant `constant`, emitting its instruction if necessary.
    /// @param constant the constant to get the ID for
    /// @returns the result ID of the constant
    uint32_t Constant(ir::Constant* constant);

    /// Get the result ID of the OpConstantNull instruction for `type`, emitting it if necessary.
    /// @param type the type to get the ID for
    /// @returns the result ID of the OpConstantNull instruction
    uint32_t ConstantNull(const type::Type* type);

    /// Get the result ID of the type `ty`, emitting a type declaration instruction if necessary.
    /// @param ty the type to get the ID for
    /// @returns the result ID of the type
    uint32_t Type(const type::Type* ty);

    /// Get the result ID of the value `value`, emitting its instruction if necessary.
    /// @param value the value to get the ID for
    /// @returns the result ID of the value
    uint32_t Value(ir::Value* value);

    /// Get the result ID of the instruction result `value`, emitting its instruction if necessary.
    /// @param inst the instruction to get the ID for
    /// @returns the result ID of the instruction
    uint32_t Value(ir::Instruction* inst);

    /// Get the ID of the label for `block`.
    /// @param block the block to get the label ID for
    /// @returns the ID of the block's label
    uint32_t Label(ir::Block* block);

    /// Emit a struct type.
    /// @param id the result ID to use
    /// @param str the struct type to emit
    void EmitStructType(uint32_t id, const type::Struct* str);

    /// Emit a function.
    /// @param func the function to emit
    void EmitFunction(ir::Function* func);

    /// Emit entry point declarations for a function.
    /// @param func the function to emit entry point declarations for
    /// @param id the result ID of the function declaration
    void EmitEntryPoint(ir::Function* func, uint32_t id);

    /// Emit a block, including the initial OpLabel, OpPhis and instructions.
    /// @param block the block to emit
    void EmitBlock(ir::Block* block);

    /// Emit all OpPhi nodes for incoming branches to @p block.
    /// @param block the block to emit the OpPhis for
    void EmitIncomingPhis(ir::MultiInBlock* block);

    /// Emit all instructions of @p block.
    /// @param block the block's instructions to emit
    void EmitBlockInstructions(ir::Block* block);

    /// Emit the root block.
    /// @param root_block the root block to emit
    void EmitRootBlock(ir::Block* root_block);

    /// Emit an `if` flow node.
    /// @param i the if node to emit
    void EmitIf(ir::If* i);

    /// Emit an access instruction
    /// @param access the access instruction to emit
    void EmitAccess(ir::Access* access);

    /// Emit a binary instruction.
    /// @param binary the binary instruction to emit
    void EmitBinary(ir::Binary* binary);

    /// Emit a builtin function call instruction.
    /// @param call the builtin call instruction to emit
    void EmitBuiltinCall(ir::BuiltinCall* call);

    /// Emit a construct instruction.
    /// @param construct the construct instruction to emit
    void EmitConstruct(ir::Construct* construct);

    /// Emit a load instruction.
    /// @param load the load instruction to emit
    void EmitLoad(ir::Load* load);

    /// Emit a loop instruction.
    /// @param loop the loop instruction to emit
    void EmitLoop(ir::Loop* loop);

    /// Emit a store instruction.
    /// @param store the store instruction to emit
    void EmitStore(ir::Store* store);

    /// Emit a switch instruction.
    /// @param swtch the switch instruction to emit
    void EmitSwitch(ir::Switch* swtch);

    /// Emit a user call instruction.
    /// @param call the user call instruction to emit
    void EmitUserCall(ir::UserCall* call);

    /// Emit a var instruction.
    /// @param var the var instruction to emit
    void EmitVar(ir::Var* var);

    /// Emit a branch instruction.
    /// @param b the branch instruction to emit
    void EmitBranch(ir::Branch* b);

    /// Emit the OpPhis for the given flow control instruction.
    /// @param inst the flow control instruction
    void EmitExitPhis(ir::ControlInstruction* inst);

  private:
    /// Get the result ID of the constant `constant`, emitting its instruction if necessary.
    /// @param constant the constant to get the ID for
    /// @returns the result ID of the constant
    uint32_t Constant(const constant::Value* constant);

    ir::Module* ir_;
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

    /// The map of constants to their result IDs.
    utils::Hashmap<const constant::Value*, uint32_t, 16> constants_;

    /// The map of types to the result IDs of their OpConstantNull instructions.
    utils::Hashmap<const type::Type*, uint32_t, 4> constant_nulls_;

    /// The map of non-constant values to their result IDs.
    utils::Hashmap<ir::Value*, uint32_t, 8> values_;

    /// The map of blocks to the IDs of their label instructions.
    utils::Hashmap<ir::Block*, uint32_t, 8> block_labels_;

    /// The map of extended instruction set names to their result IDs.
    utils::Hashmap<std::string_view, uint32_t, 2> imports_;

    /// The current function that is being emitted.
    Function current_function_;

    /// The merge block for the current if statement
    uint32_t if_merge_label_ = 0;

    /// The merge block for the current loop statement
    uint32_t loop_merge_label_ = 0;

    /// The merge block for the current switch statement
    uint32_t switch_merge_label_ = 0;

    bool zero_init_workgroup_memory_ = false;
};

}  // namespace tint::writer::spirv

#endif  // SRC_TINT_WRITER_SPIRV_IR_GENERATOR_IMPL_IR_H_
