// Copyright 2022 The Tint Authors.
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

#include "src/tint/transform/std140.h"

#include <string>
#include <utility>
#include <vector>

#include "src/tint/transform/test_helper.h"
#include "src/tint/utils/string.h"

// This file contains the should-run tests and a trival empty module test for Std140 transform.
// For testing transform results with clear readability, please refer to std140_f32_test.cc for f32
// matricies and std140_f16_test.cc for f16 matricies. For exhaustive tests that run Std140
// transform on all shape of both f32 and f16 matricies and loop on all valid literal index when
// required, please refer to std140_exhaustive_test.cc.

namespace tint::transform {
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
        str = utils::ReplaceAll(str, "${mat}", Mat());
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
}  // namespace tint::transform
