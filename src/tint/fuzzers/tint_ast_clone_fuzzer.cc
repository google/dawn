// Copyright 2020 The Dawn & Tint Authors
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

#include <iostream>
#include <string>
#include <unordered_set>

#include "src/tint/lang/wgsl/reader/parser/parser.h"
#include "src/tint/lang/wgsl/writer/writer.h"

#define ASSERT_EQ(A, B)                                        \
    do {                                                       \
        decltype(A) assert_a = (A);                            \
        decltype(B) assert_b = (B);                            \
        if (assert_a != assert_b) {                            \
            std::cerr << "ASSERT_EQ(" #A ", " #B ") failed:\n" \
                      << #A << " was: " << assert_a << "\n"    \
                      << #B << " was: " << assert_b << "\n";   \
            __builtin_trap();                                  \
        }                                                      \
    } while (false)

#define ASSERT_TRUE(A)                                                                          \
    do {                                                                                        \
        decltype(A) assert_a = (A);                                                             \
        if (!assert_a) {                                                                        \
            std::cerr << "ASSERT_TRUE(" #A ") failed:\n" << #A << " was: " << assert_a << "\n"; \
            __builtin_trap();                                                                   \
        }                                                                                       \
    } while (false)

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

    // Clone the src program to dst
    tint::Program dst(src.Clone());

    // Expect the printed strings to match
    ASSERT_EQ(tint::Program::printer(src), tint::Program::printer(dst));

    // Check that none of the AST nodes or type pointers in dst are found in src
    std::unordered_set<const tint::ast::Node*> src_nodes;
    for (auto* src_node : src.ASTNodes().Objects()) {
        src_nodes.emplace(src_node);
    }
    std::unordered_set<const tint::core::type::Type*> src_types;
    for (auto* src_type : src.Types()) {
        src_types.emplace(src_type);
    }
    for (auto* dst_node : dst.ASTNodes().Objects()) {
        ASSERT_EQ(src_nodes.count(dst_node), 0u);
    }
    for (auto* dst_type : dst.Types()) {
        ASSERT_EQ(src_types.count(dst_type), 0u);
    }

    // Regenerate the wgsl for the src program. We use this instead of the
    // original source so that reformatting doesn't impact the final wgsl
    // comparison.
    std::string src_wgsl;
    tint::wgsl::writer::Options wgsl_options;
    {
        auto result = tint::wgsl::writer::Generate(src, wgsl_options);
        ASSERT_TRUE(result == true);
        src_wgsl = result->wgsl;

        // Move the src program to a temporary that'll be dropped, so that the src
        // program is released before we attempt to print the dst program. This
        // guarantee that all the source program nodes and types are destructed and
        // freed. ASAN should error if there's any remaining references in dst when
        // we try to reconstruct the WGSL.
        auto tmp = std::move(src);
    }

    // Print the dst program, check it matches the original source
    auto result = tint::wgsl::writer::Generate(dst, wgsl_options);
    ASSERT_TRUE(result == true);
    auto dst_wgsl = result->wgsl;
    ASSERT_EQ(src_wgsl, dst_wgsl);

    return 0;
}
