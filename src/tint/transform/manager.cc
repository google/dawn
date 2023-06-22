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

#include "src/tint/transform/manager.h"

#include "src/tint/ast/transform/transform.h"
#include "src/tint/program_builder.h"

#if TINT_BUILD_IR
#include "src/tint/ir/from_program.h"
#include "src/tint/ir/to_program.h"
#include "src/tint/ir/transform/transform.h"
#else
// Declare an ir::Module class so that the transform target variant compiles.
namespace ir {
class Module;
}
#endif  // TINT_BUILD_IR

/// If set to 1 then the transform::Manager will dump the WGSL of the program
/// before and after each transform. Helpful for debugging bad output.
#define TINT_PRINT_PROGRAM_FOR_EACH_TRANSFORM 0

#if TINT_PRINT_PROGRAM_FOR_EACH_TRANSFORM
#include <iostream>
#include "src/tint/ir/disassembler.h"
#define TINT_IF_PRINT_PROGRAM(x) x
#else  // TINT_PRINT_PROGRAM_FOR_EACH_TRANSFORM
#define TINT_IF_PRINT_PROGRAM(x)
#endif  // TINT_PRINT_PROGRAM_FOR_EACH_TRANSFORM

namespace tint::transform {

Manager::Manager() = default;
Manager::~Manager() = default;

template <typename OUTPUT, typename INPUT>
OUTPUT Manager::RunTransforms(INPUT in,
                              const transform::DataMap& inputs,
                              transform::DataMap& outputs) const {
    static_assert(std::is_same<INPUT, const Program*>() || std::is_same<INPUT, ir::Module*>());
    static_assert(std::is_same<OUTPUT, Program>() || std::is_same<OUTPUT, ir::Module*>());

    // The current transform target, which could be either AST or IR.
    std::variant<const Program*, ir::Module*> target = in;
    // A local AST program to hold the result of AST transforms.
    Program ast_result;
#if TINT_BUILD_IR
    // A local IR module to hold the result of AST->IR conversions.
    ir::Module ir_result;
#endif

#if TINT_PRINT_PROGRAM_FOR_EACH_TRANSFORM
    auto print_program = [&](const char* msg, const char* name) {
        std::cout << "=========================================================" << std::endl;
        std::cout << "== " << msg << " " << name << ":" << std::endl;
        std::cout << "=========================================================" << std::endl;
        if (std::holds_alternative<const Program*>(target)) {
            auto* program = std::get<const Program*>(target);
            auto wgsl = Program::printer(program);
            std::cout << wgsl << std::endl;
            if (!program->IsValid()) {
                std::cout << "-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --"
                          << std::endl;
                std::cout << program->Diagnostics().str() << std::endl;
            }
        } else if (std::holds_alternative<ir::Module*>(target)) {
#if TINT_BUILD_IR
            ir::Disassembler dis(*std::get<ir::Module*>(target));
            std::cout << dis.Disassemble();
#endif  // TINT_BUILD_IR
        }
        std::cout << "=========================================================" << std::endl
                  << std::endl;
    };
#endif

    // Helper functions to get the current program state as either an AST program or IR module,
    // performing a conversion if necessary.
    auto get_ast = [&] {
#if TINT_BUILD_IR
        if (std::holds_alternative<ir::Module*>(target)) {
            // Convert the IR module to an AST program.
            ast_result = ir::ToProgram(*std::get<ir::Module*>(target));
            target = &ast_result;
        }
#endif  // TINT_BUILD_IR
        TINT_ASSERT(Transform, std::holds_alternative<const Program*>(target));
        return std::get<const Program*>(target);
    };
#if TINT_BUILD_IR
    auto get_ir = [&] {
        if (std::holds_alternative<const Program*>(target)) {
            // Convert the AST program to an IR module.
            auto converted = ir::FromProgram(std::get<const Program*>(target));
            TINT_ASSERT(Transform, converted);
            ir_result = converted.Move();
            target = &ir_result;
        }
        TINT_ASSERT(Transform, std::holds_alternative<ir::Module*>(target));
        return std::get<ir::Module*>(target);
    };
#endif  // TINT_BUILD_IR

    TINT_IF_PRINT_PROGRAM(print_program("Input of", "transform manager"));

    for (const auto& transform : transforms_) {
        if (auto* ast_transform = transform->As<ast::transform::Transform>()) {
            if (auto result = ast_transform->Apply(get_ast(), inputs, outputs)) {
                ast_result = std::move(result.value());
                target = &ast_result;

                if (!ast_result.IsValid()) {
                    TINT_IF_PRINT_PROGRAM(
                        print_program("Invalid output of", transform->TypeInfo().name));
                    break;
                }

                TINT_IF_PRINT_PROGRAM(print_program("Output of", transform->TypeInfo().name));
            } else {
                TINT_IF_PRINT_PROGRAM(std::cout << "Skipped " << transform->TypeInfo().name
                                                << std::endl);
            }
#if TINT_BUILD_IR
        } else if (auto* ir_transform = transform->As<ir::transform::Transform>()) {
            ir_transform->Run(get_ir(), inputs, outputs);
            TINT_IF_PRINT_PROGRAM(print_program("Output of", transform->TypeInfo().name));
#endif  // TINT_BUILD_IR
        } else {
            TINT_ASSERT(Transform, false && "unhandled transform type");
        }
    }

    TINT_IF_PRINT_PROGRAM(print_program("Final output of", "transform manager"));

    if constexpr (std::is_same<OUTPUT, Program>()) {
        auto* result = get_ast();
        if (result == in) {
            // AST transform pipelines are expected to return a clone of the program, so make sure
            // the input is cloned at least once even if nothing changed.
            ProgramBuilder b;
            CloneContext ctx{&b, result, /* auto_clone_symbols */ true};
            ctx.Clone();
            ast_result = Program(std::move(b));
        }
        return ast_result;
#if TINT_BUILD_IR
    } else if constexpr (std::is_same<OUTPUT, ir::Module*>()) {
        auto* result = get_ir();
        if (result == &ir_result) {
            // IR transform pipelines are expected to mutate the module in place, so move the local
            // temporary result to the original input.
            *in = std::move(ir_result);
        }
        return in;
#endif  // TINT_BUILD_IR
    }
}

Program Manager::Run(const Program* program,
                     const transform::DataMap& inputs,
                     transform::DataMap& outputs) const {
    return RunTransforms<Program>(program, inputs, outputs);
}

#if TINT_BUILD_IR
void Manager::Run(ir::Module* module, const DataMap& inputs, DataMap& outputs) const {
    auto* output = RunTransforms<ir::Module*>(module, inputs, outputs);
    TINT_ASSERT(Transform, output == module);
}
#endif  // TINT_BUILD_IR

}  // namespace tint::transform
