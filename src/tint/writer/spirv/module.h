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

#ifndef SRC_TINT_WRITER_SPIRV_MODULE_H_
#define SRC_TINT_WRITER_SPIRV_MODULE_H_

#include <cstdint>
#include <functional>
#include <unordered_set>
#include <vector>

#include "src/tint/writer/spirv/function.h"
#include "src/tint/writer/spirv/instruction.h"

namespace tint::writer::spirv {

/// A SPIR-V module.
class Module {
  public:
    /// Constructor
    Module();

    /// Destructor
    ~Module();

    /// @returns the number of uint32_t's needed to make up the results
    uint32_t TotalSize() const;

    /// @returns the id bound for this program
    uint32_t IdBound() const { return next_id_; }

    /// @returns the next id to be used
    uint32_t NextId() {
        auto id = next_id_;
        next_id_ += 1;
        return id;
    }

    /// Iterates over all the instructions in the correct order and calls the given callback.
    /// @param cb the callback to execute
    void Iterate(std::function<void(const Instruction&)> cb) const;

    /// Add an instruction to the list of capabilities, if the capability hasn't already been added.
    /// @param cap the capability to set
    void PushCapability(uint32_t cap);

    /// @returns the capabilities
    const InstructionList& Capabilities() const { return capabilities_; }

    /// Add an instruction to the list of extensions.
    /// @param extension the name of the extension
    void PushExtension(const char* extension);

    /// @returns the extensions
    const InstructionList& Extensions() const { return extensions_; }

    /// Add an instruction to the list of imported extension instructions.
    /// @param op the op to set
    /// @param operands the operands for the instruction
    void PushExtImport(spv::Op op, const OperandList& operands) {
        ext_imports_.push_back(Instruction{op, operands});
    }

    /// @returns the ext imports
    const InstructionList& ExtImports() const { return ext_imports_; }

    /// Add an instruction to the memory model.
    /// @param op the op to set
    /// @param operands the operands for the instruction
    void PushMemoryModel(spv::Op op, const OperandList& operands) {
        memory_model_.push_back(Instruction{op, operands});
    }

    /// @returns the memory model
    const InstructionList& MemoryModel() const { return memory_model_; }

    /// Add an instruction to the list pf entry points.
    /// @param op the op to set
    /// @param operands the operands for the instruction
    void PushEntryPoint(spv::Op op, const OperandList& operands) {
        entry_points_.push_back(Instruction{op, operands});
    }
    /// @returns the entry points
    const InstructionList& EntryPoints() const { return entry_points_; }

    /// Add an instruction to the execution mode declarations.
    /// @param op the op to set
    /// @param operands the operands for the instruction
    void PushExecutionMode(spv::Op op, const OperandList& operands) {
        execution_modes_.push_back(Instruction{op, operands});
    }

    /// @returns the execution modes
    const InstructionList& ExecutionModes() const { return execution_modes_; }

    /// Add an instruction to the debug declarations.
    /// @param op the op to set
    /// @param operands the operands for the instruction
    void PushDebug(spv::Op op, const OperandList& operands) {
        debug_.push_back(Instruction{op, operands});
    }

    /// @returns the debug instructions
    const InstructionList& Debug() const { return debug_; }

    /// Add an instruction to the type declarations.
    /// @param op the op to set
    /// @param operands the operands for the instruction
    void PushType(spv::Op op, const OperandList& operands) {
        types_.push_back(Instruction{op, operands});
    }

    /// @returns the type instructions
    const InstructionList& Types() const { return types_; }

    /// Add an instruction to the annotations.
    /// @param op the op to set
    /// @param operands the operands for the instruction
    void PushAnnot(spv::Op op, const OperandList& operands) {
        annotations_.push_back(Instruction{op, operands});
    }

    /// @returns the annotations
    const InstructionList& Annots() const { return annotations_; }

    /// Add a function to the module.
    /// @param func the function to add
    void PushFunction(const Function& func) { functions_.push_back(func); }

    /// @returns the functions
    const std::vector<Function>& Functions() const { return functions_; }

  private:
    uint32_t next_id_ = 1;
    InstructionList capabilities_;
    InstructionList extensions_;
    InstructionList ext_imports_;
    InstructionList memory_model_;
    InstructionList entry_points_;
    InstructionList execution_modes_;
    InstructionList debug_;
    InstructionList types_;
    InstructionList annotations_;
    std::vector<Function> functions_;
    std::unordered_set<uint32_t> capability_set_;
};

}  // namespace tint::writer::spirv

#endif  // SRC_TINT_WRITER_SPIRV_MODULE_H_
