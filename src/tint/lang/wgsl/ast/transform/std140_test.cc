// Copyright 2022 The Dawn & Tint Authors
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

#include "src/tint/lang/wgsl/ast/transform/std140.h"

#include <string>
#include <utility>
#include <vector>

#include "src/tint/lang/wgsl/ast/transform/helper_test.h"
#include "src/tint/utils/text/string.h"

// This file contains the should-run tests and a trival empty module test for Std140 transform.
// For testing transform results with clear readability, please refer to std140_f32_test.cc for f32
// matricies and std140_f16_test.cc for f16 matricies. For exhaustive tests that run Std140
// transform on all shape of both f32 and f16 matricies and loop on all valid literal index when
// required, please refer to std140_exhaustive_test.cc.

namespace tint::ast::transform {
namespace {

using Std140Test = TransformTest;

TEST_F(Std140Test, ShouldRunEmptyModule) {
    auto* src = R"()";

    EXPECT_FALSE(ShouldRun<Std140>(src));
}

TEST_F(Std140Test, ShouldRunStructMat2x2F32Unused) {
    auto* src = R"(
struct Unused {
  m : mat2x2<f32>,
}
)";

    EXPECT_FALSE(ShouldRun<Std140>(src));
}

TEST_F(Std140Test, ShouldRunStructMat2x2F16Unused) {
    auto* src = R"(
enable f16;

struct Unused {
  m : mat2x2<f16>,
}
)";

    EXPECT_FALSE(ShouldRun<Std140>(src));
}

enum class MatrixType { f32, f16 };

struct MatrixCase {
    uint32_t columns;
    uint32_t rows;
    MatrixType type;

    size_t ElementSize() const { return type == MatrixType::f16 ? 2 : 4; }

    size_t ColumnVectorAlign() const { return (rows == 3 ? 4 : rows) * ElementSize(); }

    bool NotStd140Compatible() const { return ColumnVectorAlign() != 16; }

    // Return if this matrix type can be used as element type of an uniform buffer, i.e. the
    // array stride is multiple of 16.
    bool CanBeUsedAsUniformArrayElememts() const {
        const size_t arrayStride = columns * ColumnVectorAlign();
        return (arrayStride % 16 == 0);
    }

    std::string Shape() const { return std::to_string(columns) + "x" + std::to_string(rows); }

    std::string ElementType() const { return type == MatrixType::f16 ? "f16" : "f32"; }

    std::string Mat() const { return "mat" + Shape() + "<" + ElementType() + ">"; }

    // Replace predefined field `${mat}` with the matrix shape. E.g. for a matrix mat4x3<f32>, would
    // replace "${mat}" with "mat4x3<f32>".
    std::string ReplaceMatInString(std::string str) const {
        str = tint::ReplaceAll(str, "${mat}", Mat());
        return str;
    }
};

inline std::ostream& operator<<(std::ostream& os, const MatrixCase& c) {
    return os << c.Mat();
}

using Std140TestShouldRun = TransformTestWithParam<MatrixCase>;

TEST_P(Std140TestShouldRun, StructStorage) {
    std::string src = R"(
enable f16;

struct S {
  m : ${mat},
}

@group(0) @binding(0) var<storage> s : S;
)";

    src = GetParam().ReplaceMatInString(src);

    EXPECT_FALSE(ShouldRun<Std140>(src));
}

TEST_P(Std140TestShouldRun, StructUniform) {
    std::string src = R"(
enable f16;

struct S {
  m : ${mat},
}

@group(0) @binding(0) var<uniform> s : S;
)";

    src = GetParam().ReplaceMatInString(src);

    EXPECT_EQ(ShouldRun<Std140>(src), GetParam().NotStd140Compatible());
}

TEST_P(Std140TestShouldRun, ArrayStorage) {
    std::string src = R"(
enable f16;

@group(0) @binding(0) var<storage> s : array<${mat}, 2>;
)";

    src = GetParam().ReplaceMatInString(src);

    EXPECT_FALSE(ShouldRun<Std140>(src));
}

TEST_P(Std140TestShouldRun, ArrayUniform) {
    auto matrix = GetParam();

    if (!matrix.CanBeUsedAsUniformArrayElememts()) {
        // This permutation is invalid, skip the test.
        return;
    }

    std::string src = R"(
enable f16;

@group(0) @binding(0) var<uniform> s : array<${mat}, 2>;
)";

    src = GetParam().ReplaceMatInString(src);

    EXPECT_EQ(ShouldRun<Std140>(src), matrix.NotStd140Compatible());
}

INSTANTIATE_TEST_SUITE_P(Std140TestShouldRun,
                         Std140TestShouldRun,
                         ::testing::ValuesIn(std::vector<MatrixCase>{
                             {2, 2, MatrixType::f32},
                             {2, 3, MatrixType::f32},
                             {2, 4, MatrixType::f32},
                             {3, 2, MatrixType::f32},
                             {3, 3, MatrixType::f32},
                             {3, 4, MatrixType::f32},
                             {4, 2, MatrixType::f32},
                             {4, 3, MatrixType::f32},
                             {4, 4, MatrixType::f32},
                             {2, 2, MatrixType::f16},
                             {2, 3, MatrixType::f16},
                             {2, 4, MatrixType::f16},
                             {3, 2, MatrixType::f16},
                             {3, 3, MatrixType::f16},
                             {3, 4, MatrixType::f16},
                             {4, 2, MatrixType::f16},
                             {4, 3, MatrixType::f16},
                             {4, 4, MatrixType::f16},
                         }));

TEST_F(Std140Test, EmptyModule) {
    auto* src = R"()";

    auto* expect = src;

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::ast::transform
