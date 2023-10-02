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

#include "src/tint/cmd/fuzz/wgsl/wgsl_fuzz.h"

#include <iostream>

#include "src/tint/lang/wgsl/reader/reader.h"
#include "src/tint/utils/containers/vector.h"
#include "src/tint/utils/macros/defer.h"
#include "src/tint/utils/macros/static_init.h"

namespace tint::fuzz::wgsl {
namespace {

Vector<ProgramFuzzer, 32> fuzzers;
std::string_view currently_running;

[[noreturn]] void TintInternalCompilerErrorReporter(const tint::InternalCompilerError& err) {
    std::cerr << "ICE while running fuzzer: '" << currently_running << "'" << std::endl;
    std::cerr << err.Error() << std::endl;
    __builtin_trap();
}

}  // namespace

void Register(const ProgramFuzzer& fuzzer) {
    fuzzers.Push(fuzzer);
}

void Run(std::string_view wgsl) {
    tint::SetInternalCompilerErrorReporter(&TintInternalCompilerErrorReporter);

    // Ensure that fuzzers are sorted. Without this, the fuzzers may be registered in any order,
    // leading to non-determinism, which we must avoid.
    TINT_STATIC_INIT(fuzzers.Sort([](auto& a, auto& b) { return a.name < b.name; }));

    // Create a Source::File to hand to the parser.
    tint::Source::File file("test.wgsl", wgsl);

    // Parse the WGSL program.
    auto program = tint::wgsl::reader::Parse(&file);
    if (!program.IsValid()) {
        return;
    }

    // Run each of the program fuzzer functions
    TINT_DEFER(currently_running = "");
    for (auto& fuzzer : fuzzers) {
        currently_running = fuzzer.name;
        fuzzer.fn(program);
    }
}

}  // namespace tint::fuzz::wgsl
