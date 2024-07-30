// Copyright 2023 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/tint/cmd/bench/bench.h"

#include <iostream>
#include <utility>

#include "src/tint/lang/spirv/reader/reader.h"
#include "src/tint/lang/wgsl/reader/reader.h"
#include "src/tint/lang/wgsl/writer/writer.h"
#include "src/tint/utils/containers/hashmap.h"

namespace tint::bench {
namespace {

// A map from benchmark input name to the corresponding WGSL shader.
Hashmap<std::string, std::string, 16> kBenchmarkWgslShaders;

}  // namespace

bool Initialize() {
    // Populate the map from benchmark input name to WGSL shader.
    for (auto& benchmark : kBenchmarkInputs) {
        if (!benchmark.wgsl.empty()) {
            // If the input is WGSL, we just add it as is.
            kBenchmarkWgslShaders.Add(benchmark.name, benchmark.wgsl);
        } else if (!benchmark.spirv.empty()) {
            // If the input is SPIR-V, we convert it to WGSL and add that.
            tint::spirv::reader::Options spirv_opts;
            spirv_opts.allow_non_uniform_derivatives = true;
            auto program = tint::spirv::reader::Read(benchmark.spirv, spirv_opts);
            if (!program.IsValid()) {
                std::cerr << "Failed to convert '" << benchmark.name
                          << "': " << program.Diagnostics() << "\n";
                return false;
            }
            auto result = tint::wgsl::writer::Generate(program, {});
            if (result != Success) {
                std::cerr << "Failed to generate WGSL for '" << benchmark.name
                          << "': " << result.Failure() << "\n";
                return false;
            }
            kBenchmarkWgslShaders.Add(benchmark.name, result->wgsl);
        } else {
            TINT_UNREACHABLE();
        }
    }

    return true;
}

Result<Source::File> GetWgslFile(std::string name) {
    auto wgsl = kBenchmarkWgslShaders.GetOr(name, "");
    if (wgsl.empty()) {
        return Failure{"failed to find WGSL shader for '" + name + "'"};
    }
    return tint::Source::File("<input>", wgsl);
}

Result<ProgramAndFile> GetWgslProgram(std::string name) {
    auto res = GetWgslFile(name);
    if (res != Success) {
        return res.Failure();
    }
    auto file = std::make_unique<Source::File>(res.Get());
    auto program = wgsl::reader::Parse(file.get());
    if (!program.IsValid()) {
        return Failure{program.Diagnostics()};
    }
    return ProgramAndFile{std::move(program), std::move(file)};
}

}  // namespace tint::bench
