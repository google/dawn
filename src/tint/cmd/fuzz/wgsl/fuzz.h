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

#ifndef SRC_TINT_CMD_FUZZ_WGSL_FUZZ_H_
#define SRC_TINT_CMD_FUZZ_WGSL_FUZZ_H_

#include <string>
#include <tuple>
#include <utility>

#include "src/tint/lang/wgsl/program/program.h"
#include "src/tint/utils/bytes/decoder.h"
#include "src/tint/utils/containers/slice.h"
#include "src/tint/utils/macros/static_init.h"
#include "src/tint/utils/reflection/reflection.h"

namespace tint::fuzz::wgsl {

/// ProgramFuzzer describes a fuzzer function that takes a WGSL program as input
struct ProgramFuzzer {
    /// @param name the name of the fuzzer
    /// @param fn the fuzzer function
    /// @returns a ProgramFuzzer that invokes the function @p fn with the Program, along with any
    /// additional arguments which are deserialized from the fuzzer input.
    template <typename... ARGS>
    static ProgramFuzzer Create(std::string_view name, void (*fn)(const Program&, ARGS...)) {
        if constexpr (sizeof...(ARGS) > 0) {
            auto fn_with_decode = [fn](const Program& program, Slice<const std::byte> data) {
                if (!data.data) {
                    return;
                }
                bytes::BufferReader reader{data};
                if (auto data_args = bytes::Decode<std::tuple<std::decay_t<ARGS>...>>(reader)) {
                    auto all_args =
                        std::tuple_cat(std::tuple<const Program&>{program}, data_args.Get());
                    std::apply(*fn, all_args);
                }
            };
            return ProgramFuzzer{name, std::move(fn_with_decode)};
        } else {
            return ProgramFuzzer{
                name,
                [fn](const Program& program, Slice<const std::byte>) { fn(program); },
            };
        }
    }

    /// Name of the fuzzer function
    std::string_view name;
    /// The fuzzer function
    std::function<void(const Program&, Slice<const std::byte> data)> fn;
};

/// Options for Run()
struct Options {
    /// If true, the fuzzers will be run concurrently on separate threads.
    bool run_concurrently = false;
};

/// Runs all the registered WGSL fuzzers with the supplied WGSL
/// @param wgsl the input WGSL
/// @param data additional data used for fuzzing
/// @param options the options for running the fuzzers
void Run(std::string_view wgsl, Slice<const std::byte> data, const Options& options);

/// Registers the fuzzer function with the WGSL fuzzer executable.
/// @param fuzzer the fuzzer
void Register(const ProgramFuzzer& fuzzer);

/// TINT_WGSL_PROGRAM_FUZZER registers the fuzzer function to run as part of `tint_wgsl_fuzzer`
#define TINT_WGSL_PROGRAM_FUZZER(FUNCTION)         \
    TINT_STATIC_INIT(::tint::fuzz::wgsl::Register( \
        ::tint::fuzz::wgsl::ProgramFuzzer::Create(#FUNCTION, FUNCTION)))

}  // namespace tint::fuzz::wgsl

#endif  // SRC_TINT_CMD_FUZZ_WGSL_FUZZ_H_
