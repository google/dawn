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

#include "src/tint/ir/from_program.h"
#include "src/tint/ir/to_program.h"
#include "src/tint/reader/wgsl/parser_impl.h"
#include "src/tint/writer/wgsl/generator.h"

[[noreturn]] void TintInternalCompilerErrorReporter(const tint::diag::List& diagnostics) {
    auto printer = tint::diag::Printer::create(stderr, true);
    tint::diag::Formatter{}.format(diagnostics, printer.get());
    __builtin_trap();
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    std::string str(reinterpret_cast<const char*>(data), size);

    tint::SetInternalCompilerErrorReporter(&TintInternalCompilerErrorReporter);

    tint::Source::File file("test.wgsl", str);

    // Parse the wgsl, create the src program
    tint::reader::wgsl::ParserImpl parser(&file);
    parser.set_max_errors(1);
    if (!parser.Parse()) {
        return 0;
    }
    auto src = parser.program();
    if (!src.IsValid()) {
        return 0;
    }

    auto ir = tint::ir::FromProgram(&src);
    if (!ir) {
        std::cerr << ir.Failure() << std::endl;
        __builtin_trap();
    }

    auto dst = tint::ir::ToProgram(ir.Get());
    if (!dst.IsValid()) {
#if TINT_BUILD_WGSL_WRITER
        if (auto result = tint::writer::wgsl::Generate(&dst, {}); result.success) {
            std::cerr << result.wgsl << std::endl << std::endl;
        }
#endif

        std::cerr << dst.Diagnostics() << std::endl;
        __builtin_trap();
    }

    return 0;
}
