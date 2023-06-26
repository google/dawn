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

#include "src/tint/writer/msl/ir/generator_impl_ir.h"

#include "src/tint/ir/validate.h"
#include "src/tint/switch.h"
#include "src/tint/transform/manager.h"
#include "src/tint/type/bool.h"
#include "src/tint/type/f16.h"
#include "src/tint/type/f32.h"
#include "src/tint/type/i32.h"
#include "src/tint/type/u32.h"
#include "src/tint/type/void.h"
#include "src/tint/utils/scoped_assignment.h"

namespace tint::writer::msl {
namespace {

void Sanitize(ir::Module* module) {
    transform::Manager manager;
    transform::DataMap data;

    transform::DataMap outputs;
    manager.Run(module, data, outputs);
}

}  // namespace

// Helper for calling TINT_UNIMPLEMENTED() from a Switch(object_ptr) default case.
#define UNHANDLED_CASE(object_ptr)           \
    TINT_UNIMPLEMENTED(Writer, diagnostics_) \
        << "unhandled case in Switch(): " << (object_ptr ? object_ptr->TypeInfo().name : "<null>")

GeneratorImplIr::GeneratorImplIr(ir::Module* module) : IRTextGenerator(module) {}

GeneratorImplIr::~GeneratorImplIr() = default;

bool GeneratorImplIr::Generate() {
    auto valid = ir::Validate(*ir_);
    if (!valid) {
        diagnostics_ = valid.Failure();
        return false;
    }

    // Run the IR transformations to prepare for MSL emission.
    Sanitize(ir_);

    {
        TINT_SCOPED_ASSIGNMENT(current_buffer_, &preamble_buffer_);
        Line() << "#include <metal_stdlib>";
        Line();
        Line() << "using namespace metal;";
    }

    // Emit module-scope declarations.
    if (ir_->root_block) {
        // EmitRootBlock(ir_->root_block);
    }

    // Emit functions.
    for (auto* func : ir_->functions) {
        EmitFunction(func);
    }

    if (diagnostics_.contains_errors()) {
        return false;
    }

    return true;
}

void GeneratorImplIr::EmitFunction(ir::Function* func) {
    {
        auto out = Line();
        EmitType(out, func->ReturnType());
        out << " " << ir_->NameOf(func).Name() << "() {";
    }
    Line() << "}";
}

void GeneratorImplIr::EmitType(utils::StringStream& out, const type::Type* ty) {
    tint::Switch(
        ty,                                         //
        [&](const type::Bool*) { out << "bool"; },  //
        [&](const type::Void*) { out << "void"; },  //
        [&](const type::F32*) { out << "float"; },  //
        [&](const type::F16*) { out << "half"; },   //
        [&](const type::I32*) { out << "int"; },    //
        [&](const type::U32*) { out << "uint"; },   //
        [&](Default) { UNHANDLED_CASE(ty); });
}

}  // namespace tint::writer::msl
