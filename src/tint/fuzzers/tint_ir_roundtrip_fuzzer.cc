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

#include <iostream>
#include <string>
#include <unordered_set>

#include "src/tint/fuzzers/apply_substitute_overrides.h"
#include "src/tint/lang/wgsl/reader/parser/parser.h"
#include "src/tint/lang/wgsl/reader/program_to_ir/program_to_ir.h"
#include "src/tint/lang/wgsl/writer/ir_to_program/ir_to_program.h"
#include "src/tint/lang/wgsl/writer/writer.h"

[[noreturn]] void TintInternalCompilerErrorReporter(const tint::InternalCompilerError& err) {
    std::cerr << err.Error() << std::endl;
    __builtin_trap();
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    std::string str(reinterpret_cast<const char*>(data), size);

    tint::SetInternalCompilerErrorReporter(&TintInternalCompilerErrorReporter);

    tint::Source::File file("test.wgsl", str);

    // Parse the wgsl, create the src program
    tint::wgsl::reader::Parser parser(&file);
    parser.set_max_errors(1);
    if (!parser.Parse()) {
        return 0;
    }
    auto src = parser.program();
    if (!src.IsValid()) {
        return 0;
    }

    auto is_unsupported = [](const tint::ast::Enable* enable) {
        for (auto ext : enable->extensions) {
            switch (ext->name) {
                case tint::builtin::Extension::kChromiumExperimentalDp4A:
                case tint::builtin::Extension::kChromiumExperimentalFullPtrParameters:
                case tint::builtin::Extension::kChromiumExperimentalPushConstant:
                case tint::builtin::Extension::kChromiumInternalDualSourceBlending:
                case tint::builtin::Extension::kChromiumInternalRelaxedUniformLayout:
                    return true;
                default:
                    break;
            }
        }
        return false;
    };

    if (src.AST().Enables().Any(is_unsupported)) {
        return 0;
    }

    src = tint::fuzzers::ApplySubstituteOverrides(std::move(src));
    if (!src.IsValid()) {
        return 0;
    }

    auto ir = tint::wgsl::reader::ProgramToIR(&src);
    if (!ir) {
        std::cerr << ir.Failure() << std::endl;
        __builtin_trap();
    }

    auto dst = tint::wgsl::writer::IRToProgram(ir.Get());
    if (!dst.IsValid()) {
#if TINT_BUILD_WGSL_WRITER
        if (auto result = tint::wgsl::writer::Generate(&dst, {}); result) {
            std::cerr << result->wgsl << std::endl << std::endl;
        }
#endif

        std::cerr << dst.Diagnostics() << std::endl;
        __builtin_trap();
    }

    return 0;
}
