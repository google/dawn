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

#include <iostream>
#include <string>
#include <unordered_set>

#include "src/tint/reader/wgsl/parser_impl.h"
#include "src/tint/writer/wgsl/generator.h"

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

    // Clone the src program to dst
    tint::Program dst(src.Clone());

    // Expect the printed strings to match
    ASSERT_EQ(tint::Program::printer(&src), tint::Program::printer(&dst));

    // Check that none of the AST nodes or type pointers in dst are found in src
    std::unordered_set<const tint::ast::Node*> src_nodes;
    for (auto* src_node : src.ASTNodes().Objects()) {
        src_nodes.emplace(src_node);
    }
    std::unordered_set<const tint::type::Type*> src_types;
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
    tint::writer::wgsl::Options wgsl_options;
    {
        auto result = tint::writer::wgsl::Generate(&src, wgsl_options);
        ASSERT_TRUE(result.success);
        src_wgsl = result.wgsl;

        // Move the src program to a temporary that'll be dropped, so that the src
        // program is released before we attempt to print the dst program. This
        // guarantee that all the source program nodes and types are destructed and
        // freed. ASAN should error if there's any remaining references in dst when
        // we try to reconstruct the WGSL.
        auto tmp = std::move(src);
    }

    // Print the dst program, check it matches the original source
    auto result = tint::writer::wgsl::Generate(&dst, wgsl_options);
    ASSERT_TRUE(result.success);
    auto dst_wgsl = result.wgsl;
    ASSERT_EQ(src_wgsl, dst_wgsl);

    return 0;
}
