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

#include "src/tint/transform/vectorize_matrix_conversions.h"

#include <string>
#include <utility>

#include "src/tint/transform/test_helper.h"
#include "src/tint/utils/string.h"

namespace tint::transform {
namespace {

using VectorizeMatrixConversionsTest = TransformTestWithParam<std::pair<uint32_t, uint32_t>>;

TEST_F(VectorizeMatrixConversionsTest, ShouldRunEmptyModule) {
    auto* src = R"()";

    EXPECT_FALSE(ShouldRun<VectorizeMatrixConversions>(src));
}

// Test that VectorizeMatrixConversions transforms the matRxC<f32> to matRxC<f16> conversion as
// expected.
//
// Example input:
//
//   enable f16;
//
//   @fragment
//   fn main() {
//     let m = mat3x2<f32>(vec2<f32>(0.0, 1.0), vec2<f32>(2.0, 3.0), vec2<f32>(4.0, 5.0));
//     let n : mat3x2<f16> = mat3x2<f16>(m);
//   }
//
// Example output:
//
//   enable f16;
//
//   @fragment
//   fn main() {
//     let m = mat3x2<f32>(vec2<f32>(0.0, 1.0), vec2<f32>(2.0, 3.0), vec2<f32>(4.0, 5.0));
//     let n : mat3x2<f16> = mat3x2<f16>(vec2<f16>(m[0]), vec2<f16>(m[1]), vec2<f16>(m[2]));
//   }
TEST_P(VectorizeMatrixConversionsTest, Conversion_F32ToF16) {
    uint32_t cols = GetParam().first;
    uint32_t rows = GetParam().second;
    std::string src_mat_type = "mat" + std::to_string(cols) + "x" + std::to_string(rows) + "<f32>";
    std::string src_vec_type = "vec" + std::to_string(rows) + "<f32>";
    std::string dst_mat_type = "mat" + std::to_string(cols) + "x" + std::to_string(rows) + "<f16>";
    std::string dst_vec_type = "vec" + std::to_string(rows) + "<f16>";
    std::string vector_values;
    for (uint32_t c = 0; c < cols; c++) {
        if (c > 0) {
            vector_values += ", ";
        }
        vector_values += src_vec_type + "(";
        for (uint32_t r = 0; r < rows; r++) {
            if (r > 0) {
                vector_values += ", ";
            }
            auto value = std::to_string(c * rows + r) + ".0";
            vector_values += value;
        }
        vector_values += ")";
    }

    std::string vectorized_args = "";
    for (uint32_t c = 0; c < cols; c++) {
        if (c > 0) {
            vectorized_args += ", ";
        }
        vectorized_args += dst_vec_type + "(m[" + std::to_string(c) + "])";
    }

    std::string tmpl = R"(
enable f16;

@fragment
fn main() {
  let m = ${src_mat_type}(${values});
  let n : ${dst_mat_type} = ${dst_mat_type}(${args});
}
)";
    tmpl = utils::ReplaceAll(tmpl, "${src_mat_type}", src_mat_type);
    tmpl = utils::ReplaceAll(tmpl, "${dst_mat_type}", dst_mat_type);
    tmpl = utils::ReplaceAll(tmpl, "${values}", vector_values);
    auto src = utils::ReplaceAll(tmpl, "${args}", "m");
    auto expect = utils::ReplaceAll(tmpl, "${args}", vectorized_args);

    EXPECT_TRUE(ShouldRun<VectorizeMatrixConversions>(src));

    auto got = Run<VectorizeMatrixConversions>(src);

    EXPECT_EQ(expect, str(got));
}

// Test that VectorizeMatrixConversions transforms the matRxC<f32> to matRxC<f16> conversion as
// expected.
//
// Example input:
//
//   enable f16;
//
//   @fragment
//   fn main() {
//     let m = mat3x2<f16>(vec2<f16>(0.0, 1.0), vec2<f16>(2.0, 3.0), vec2<f16>(4.0, 5.0));
//     let n : mat3x2<f32> = mat3x2<f32>(m);
//   }
//
// Example output:
//
//   enable f16;
//
//   @fragment
//   fn main() {
//     let m = mat3x2<f16>(vec2<f16>(0.0, 1.0), vec2<f16>(2.0, 3.0), vec2<f16>(4.0, 5.0));
//     let n : mat3x2<f32> = mat3x2<f32>(vec2<f32>(m[0]), vec2<f32>(m[1]), vec2<f32>(m[2]));
//   }
TEST_P(VectorizeMatrixConversionsTest, Conversion_F16ToF32) {
    uint32_t cols = GetParam().first;
    uint32_t rows = GetParam().second;
    std::string src_mat_type = "mat" + std::to_string(cols) + "x" + std::to_string(rows) + "<f16>";
    std::string src_vec_type = "vec" + std::to_string(rows) + "<f16>";
    std::string dst_mat_type = "mat" + std::to_string(cols) + "x" + std::to_string(rows) + "<f32>";
    std::string dst_vec_type = "vec" + std::to_string(rows) + "<f32>";
    std::string vector_values;
    for (uint32_t c = 0; c < cols; c++) {
        if (c > 0) {
            vector_values += ", ";
        }
        vector_values += src_vec_type + "(";
        for (uint32_t r = 0; r < rows; r++) {
            if (r > 0) {
                vector_values += ", ";
            }
            auto value = std::to_string(c * rows + r) + ".0";
            vector_values += value;
        }
        vector_values += ")";
    }

    std::string vectorized_args = "";
    for (uint32_t c = 0; c < cols; c++) {
        if (c > 0) {
            vectorized_args += ", ";
        }
        vectorized_args += dst_vec_type + "(m[" + std::to_string(c) + "])";
    }

    std::string tmpl = R"(
enable f16;

@fragment
fn main() {
  let m = ${src_mat_type}(${values});
  let n : ${dst_mat_type} = ${dst_mat_type}(${args});
}
)";
    tmpl = utils::ReplaceAll(tmpl, "${src_mat_type}", src_mat_type);
    tmpl = utils::ReplaceAll(tmpl, "${dst_mat_type}", dst_mat_type);
    tmpl = utils::ReplaceAll(tmpl, "${values}", vector_values);
    auto src = utils::ReplaceAll(tmpl, "${args}", "m");
    auto expect = utils::ReplaceAll(tmpl, "${args}", vectorized_args);

    EXPECT_TRUE(ShouldRun<VectorizeMatrixConversions>(src));

    auto got = Run<VectorizeMatrixConversions>(src);

    EXPECT_EQ(expect, str(got));
}

// Test that VectorizeMatrixConversions transform generates help functions for conversions of which
// input expression has side effect.
//
// Example input:
//
//   enable f16;
//
//   var<private> i : i32 = 0;
//
//   fn mat_f32() -> mat2x2<f32> {
//     i = (i + 1);
//     return mat2x2<f32>(vec2<f32>(f32(i), f32(i)), vec2<f32>(f32(i), f32(i)));
//   }
//
//   fn mat_f16() -> mat2x2<f16> {
//     i = (i + 1);
//     return mat2x2<f16>(vec2<f16>(f16(i), f16(i)), vec2<f16>(f16(i), f16(i)));
//   }
//
//   @fragment
//   fn main() {
//     let m32 : mat2x2<f32> = mat2x2<f32>(mat_f16());
//     let m16 : mat2x2<f16> = mat2x2<f16>(mat_f32());
//   }
//
// Example output:
//
//   enable f16;
//
//   var<private> i : i32 = 0;
//
//   fn mat_f32() -> mat2x2<f32> {
//     i = (i + 1);
//     return mat2x2<f32>(vec2<f32>(f32(i), f32(i)), vec2<f32>(f32(i), f32(i)));
//   }
//
//   fn mat_f16() -> mat2x2<f16> {
//     i = (i + 1);
//     return mat2x2<f16>(vec2<f16>(f16(i), f16(i)), vec2<f16>(f16(i), f16(i)));
//   }
//
//   fn convert_mat2x2_f16_f32(value : mat2x2<f16>) -> mat2x2<f32> {
//     return mat2x2<f32>(vec2<f32>(value[0]), vec2<f32>(value[1]));
//   }
//
//   fn convert_mat2x2_f32_f16(value : mat2x2<f32>) -> mat2x2<f16> {
//     return mat2x2<f16>(vec2<f16>(value[0]), vec2<f16>(value[1]));
//   }
//
//   @fragment
//   fn main() {
//     let m32 : mat2x2<f32> = convert_mat2x2_f16_f32(mat_f16());
//     let m16 : mat2x2<f16> = convert_mat2x2_f32_f16(mat_f32());
//   }
TEST_P(VectorizeMatrixConversionsTest, Conversion_WithSideEffect) {
    uint32_t cols = GetParam().first;
    uint32_t rows = GetParam().second;
    std::string mat_shape = "mat" + std::to_string(cols) + "x" + std::to_string(rows);
    std::string f32_mat_type = mat_shape + "<f32>";
    std::string f32_vec_type = "vec" + std::to_string(rows) + "<f32>";
    std::string f16_mat_type = mat_shape + "<f16>";
    std::string f16_vec_type = "vec" + std::to_string(rows) + "<f16>";
    std::string f32_vector_values;
    std::string f16_vector_values;
    for (uint32_t c = 0; c < cols; c++) {
        if (c > 0) {
            f32_vector_values += ", ";
            f16_vector_values += ", ";
        }
        f32_vector_values += f32_vec_type + "(";
        f16_vector_values += f16_vec_type + "(";
        for (uint32_t r = 0; r < rows; r++) {
            if (r > 0) {
                f32_vector_values += ", ";
                f16_vector_values += ", ";
            }
            f32_vector_values += "f32(i)";
            f16_vector_values += "f16(i)";
        }
        f32_vector_values += ")";
        f16_vector_values += ")";
    }

    std::string f32_vectorized_args = "";
    std::string f16_vectorized_args = "";
    for (uint32_t c = 0; c < cols; c++) {
        if (c > 0) {
            f32_vectorized_args += ", ";
            f16_vectorized_args += ", ";
        }
        f32_vectorized_args += f32_vec_type + "(value[" + std::to_string(c) + "])";
        f16_vectorized_args += f16_vec_type + "(value[" + std::to_string(c) + "])";
    }

    std::string tmpl = R"(
enable f16;

var<private> i : i32 = 0;

fn mat_f32() -> ${f32_mat_type} {
  i = (i + 1);
  return ${f32_mat_type}(${f32_values});
}

fn mat_f16() -> ${f16_mat_type} {
  i = (i + 1);
  return ${f16_mat_type}(${f16_values});
}
${helper_function}
@fragment
fn main() {
  let m32 : ${f32_mat_type} = ${f32_matrix_conversion};
  let m16 : ${f16_mat_type} = ${f16_matrix_conversion};
}
)";
    tmpl = utils::ReplaceAll(tmpl, "${f32_values}", f32_vector_values);
    tmpl = utils::ReplaceAll(tmpl, "${f16_values}", f16_vector_values);
    auto src = utils::ReplaceAll(tmpl, "${f32_matrix_conversion}", "${f32_mat_type}(mat_f16())");
    src = utils::ReplaceAll(src, "${f16_matrix_conversion}", "${f16_mat_type}(mat_f32())");
    src = utils::ReplaceAll(src, "${helper_function}", "");
    src = utils::ReplaceAll(src, "${f32_mat_type}", f32_mat_type);
    src = utils::ReplaceAll(src, "${f16_mat_type}", f16_mat_type);

    auto helper_function = std::string(R"(
fn convert_${mat_shape}_f16_f32(value : ${f16_mat_type}) -> ${f32_mat_type} {
  return ${f32_mat_type}(${f32_vectorized_args});
}

fn convert_${mat_shape}_f32_f16(value : ${f32_mat_type}) -> ${f16_mat_type} {
  return ${f16_mat_type}(${f16_vectorized_args});
}
)");
    auto expect = utils::ReplaceAll(tmpl, "${helper_function}", helper_function);
    expect = utils::ReplaceAll(expect, "${f32_mat_type}", f32_mat_type);
    expect = utils::ReplaceAll(expect, "${f16_mat_type}", f16_mat_type);
    expect = utils::ReplaceAll(expect, "${f32_matrix_conversion}",
                               "convert_${mat_shape}_f16_f32(mat_f16())");
    expect = utils::ReplaceAll(expect, "${f16_matrix_conversion}",
                               "convert_${mat_shape}_f32_f16(mat_f32())");
    expect = utils::ReplaceAll(expect, "${mat_shape}", mat_shape);
    expect = utils::ReplaceAll(expect, "${f32_vectorized_args}", f32_vectorized_args);
    expect = utils::ReplaceAll(expect, "${f16_vectorized_args}", f16_vectorized_args);

    EXPECT_TRUE(ShouldRun<VectorizeMatrixConversions>(src));

    auto got = Run<VectorizeMatrixConversions>(src);

    EXPECT_EQ(expect, str(got));
}

// Test that VectorizeMatrixConversions transform will not run for matrix initializer.
TEST_P(VectorizeMatrixConversionsTest, NonConversion_InitializerFromVectors) {
    uint32_t cols = GetParam().first;
    uint32_t rows = GetParam().second;
    std::string mat_type = "mat" + std::to_string(cols) + "x" + std::to_string(rows) + "<f32>";
    std::string vec_type = "vec" + std::to_string(rows) + "<f32>";
    std::string columns;
    for (uint32_t c = 0; c < cols; c++) {
        if (c > 0) {
            columns += ", ";
        }
        columns += vec_type + "()";
    }

    std::string tmpl = R"(
@fragment
fn main() {
  let m = ${matrix}(${columns});
}
)";
    tmpl = utils::ReplaceAll(tmpl, "${matrix}", mat_type);
    auto src = utils::ReplaceAll(tmpl, "${columns}", columns);
    auto expect = src;

    EXPECT_FALSE(ShouldRun<VectorizeMatrixConversions>(src));

    auto got = Run<VectorizeMatrixConversions>(src);

    EXPECT_EQ(expect, str(got));
}

// Test that VectorizeMatrixConversions transform will not run for identity matrix initializer,
// which also take a single matrix as input.
TEST_P(VectorizeMatrixConversionsTest, NonConversion_IdentityInitializer) {
    uint32_t cols = GetParam().first;
    uint32_t rows = GetParam().second;
    std::string mat_type = "mat" + std::to_string(cols) + "x" + std::to_string(rows) + "<f32>";
    std::string vec_type = "vec" + std::to_string(rows) + "<f32>";
    std::string columns;
    for (uint32_t c = 0; c < cols; c++) {
        if (c > 0) {
            columns += ", ";
        }
        columns += vec_type + "()";
    }

    std::string tmpl = R"(
@fragment
fn main() {
  let m = ${matrix}(${columns});
  let n : ${matrix} = ${matrix}(m);
}
)";
    tmpl = utils::ReplaceAll(tmpl, "${matrix}", mat_type);
    auto src = utils::ReplaceAll(tmpl, "${columns}", columns);
    auto expect = src;

    EXPECT_FALSE(ShouldRun<VectorizeMatrixConversions>(src));

    auto got = Run<VectorizeMatrixConversions>(src);

    EXPECT_EQ(expect, str(got));
}

INSTANTIATE_TEST_SUITE_P(VectorizeMatrixConversionsTest,
                         VectorizeMatrixConversionsTest,
                         testing::Values(std::make_pair(2, 2),
                                         std::make_pair(2, 3),
                                         std::make_pair(2, 4),
                                         std::make_pair(3, 2),
                                         std::make_pair(3, 3),
                                         std::make_pair(3, 4),
                                         std::make_pair(4, 2),
                                         std::make_pair(4, 3),
                                         std::make_pair(4, 4)));

}  // namespace
}  // namespace tint::transform
