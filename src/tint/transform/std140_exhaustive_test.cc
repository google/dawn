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

namespace tint::transform {
namespace {

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
        const size_t array_stride = columns * ColumnVectorAlign();
        return (array_stride % 16 == 0);
    }

    std::string Shape() const { return std::to_string(columns) + "x" + std::to_string(rows); }

    std::string ElementType() const { return type == MatrixType::f16 ? "f16" : "f32"; }

    std::string Mat() const { return "mat" + Shape() + "<" + ElementType() + ">"; }

    std::string ColumnVector() const {
        return "vec" + std::to_string(rows) + "<" + (type == MatrixType::f32 ? "f32" : "f16") + ">";
    }

    std::string ColumnVectorSwizzle() const {
        switch (rows) {
            case 2:
                return "yx";
            case 3:
                return "yzx";
            case 4:
                return "wzxy";
        }
        return "";
    }

    // For each column, replaces "${col_id_for_tmpl}" by column index in `tmpl` to get a string, and
    // join all these strings with `seperator`. If `tmpl_for_last_column` is not empty, use it
    // instead of `tmpl` for the last column.
    std::string JoinTemplatedStringForEachMatrixColumn(
        std::string tmpl,
        std::string seperator,
        std::string tmpl_for_last_column = "") const {
        std::string result;
        if (tmpl_for_last_column.size() == 0) {
            tmpl_for_last_column = tmpl;
        }
        for (size_t c = 0; c < columns - 1; c++) {
            if (c > 0) {
                result += seperator;
            }
            std::string string_for_current_column =
                utils::ReplaceAll(tmpl, "${col_id_for_tmpl}", std::to_string(c));
            result += string_for_current_column;
        }
        result += seperator;
        std::string string_for_last_column = utils::ReplaceAll(
            tmpl_for_last_column, "${col_id_for_tmpl}", std::to_string(columns - 1));
        result += string_for_last_column;
        return result;
    }

    std::string ExpendedColumnVectors(uint32_t leading_space, std::string name) const {
        std::string space(leading_space, ' ');
        return JoinTemplatedStringForEachMatrixColumn(
            space + name + "${col_id_for_tmpl} : " + ColumnVector() + ",", "\n");
    }

    std::string ExpendedColumnVectorsInline(std::string name, std::string seperator) const {
        return JoinTemplatedStringForEachMatrixColumn(name + "${col_id_for_tmpl}", seperator);
    }

    std::string ExpendedColumnVectorsWithLastSize(uint32_t leading_space,
                                                  std::string name,
                                                  uint32_t last_size) const {
        std::string space(leading_space, ' ');
        return JoinTemplatedStringForEachMatrixColumn(
            space + name + "${col_id_for_tmpl} : " + ColumnVector() + ",", "\n",
            space + "@size(" + std::to_string(last_size) + ")\n" + space + name +
                "${col_id_for_tmpl} : " + ColumnVector() + ",");
    }

    // Replace user-given fields and predefined fields in a given string `str`.
    // First, for each pair of string in `replacement_pairs`, replace all occurrences of the first
    // string of pair with second string. Then, replace several predefined fields with the matrix
    // information. E.g. for a matrix mat4x3<f32>, would replace "${mat}" with "mat4x3<f32>",
    // replace "${shape}" with "4x3", "${elem_type}" with "f32", "${col_vector_type}" with
    // "vec3<f32>", and "${swizzle}" with "yzx".
    std::string ReplaceFieldsInString(
        std::string str,
        std::initializer_list<std::pair<std::string, std::string>> replacement_pairs = {}) const {
        for (auto& replace : replacement_pairs) {
            str = utils::ReplaceAll(str, replace.first, replace.second);
        }
        str = utils::ReplaceAll(str, "${mat}", Mat());
        str = utils::ReplaceAll(str, "${shape}", Shape());
        str = utils::ReplaceAll(str, "${elem_type}", ElementType());
        str = utils::ReplaceAll(str, "${col_vector_type}", ColumnVector());
        str = utils::ReplaceAll(str, "${swizzle}", ColumnVectorSwizzle());
        return str;
    }
};

inline std::ostream& operator<<(std::ostream& os, const MatrixCase& c) {
    return os << c.Mat();
}

using Std140Test_Matrix = TransformTestWithParam<MatrixCase>;

TEST_P(Std140Test_Matrix, SingleStructMatUniform) {
    auto matrix = GetParam();

    std::string src = R"(
enable f16;

struct S {
  m : ${mat},
}

@group(0) @binding(0) var<uniform> s : S;
)";
    src = matrix.ReplaceFieldsInString(src);

    std::string expect;
    if (matrix.NotStd140Compatible()) {
        expect = R"(
enable f16;

struct S {
  m : ${mat},
}

struct S_std140 {
${col_vectors}
}

@group(0) @binding(0) var<uniform> s : S_std140;
)";
        expect = matrix.ReplaceFieldsInString(
            expect, {{"${col_vectors}", matrix.ExpendedColumnVectors(2, "m_")}});

    } else {
        expect = src;
    }

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_P(Std140Test_Matrix, CustomAlign) {
    auto matrix = GetParam();

    std::string src = R"(
enable f16;

struct S {
  before : i32,
  @align(128)
  m : ${mat},
  after : i32,
}

@group(0) @binding(0) var<uniform> s : S;
)";
    src = matrix.ReplaceFieldsInString(src);

    std::string expect;
    if (matrix.NotStd140Compatible()) {
        expect = R"(
enable f16;

struct S {
  before : i32,
  @align(128)
  m : ${mat},
  after : i32,
}

struct S_std140 {
  before : i32,
  @align(128i)
${col_vectors}
  after : i32,
}

@group(0) @binding(0) var<uniform> s : S_std140;
)";
        expect = matrix.ReplaceFieldsInString(
            expect, {{"${col_vectors}", matrix.ExpendedColumnVectors(2, "m_")}});
    } else {
        expect = src;
    }

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_P(Std140Test_Matrix, CustomSizeMat) {
    auto matrix = GetParam();

    std::string src = R"(
enable f16;

struct S {
  before : i32,
  @size(128)
  m : ${mat},
  after : i32,
}

@group(0) @binding(0) var<uniform> s : S;
)";
    src = matrix.ReplaceFieldsInString(src);

    std::string expect;
    if (matrix.NotStd140Compatible()) {
        uint32_t last_size =
            128 - static_cast<uint32_t>(matrix.ColumnVectorAlign() * (matrix.columns - 1));

        expect = R"(
enable f16;

struct S {
  before : i32,
  @size(128)
  m : ${mat},
  after : i32,
}

struct S_std140 {
  before : i32,
${col_vectors}
  after : i32,
}

@group(0) @binding(0) var<uniform> s : S_std140;
)";
        expect = matrix.ReplaceFieldsInString(
            expect,
            {{"${col_vectors}", matrix.ExpendedColumnVectorsWithLastSize(2, "m_", last_size)}});
    } else {
        expect = src;
    }

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_P(Std140Test_Matrix, CustomAlignAndSize) {
    auto matrix = GetParam();

    std::string src = R"(
enable f16;

struct S {
  before : i32,
  @align(128) @size(128)
  m : ${mat},
  after : i32,
}

@group(0) @binding(0) var<uniform> s : S;
)";
    src = matrix.ReplaceFieldsInString(src);

    std::string expect;
    if (matrix.NotStd140Compatible()) {
        uint32_t last_size =
            128 - static_cast<uint32_t>(matrix.ColumnVectorAlign() * (matrix.columns - 1));

        expect = R"(
enable f16;

struct S {
  before : i32,
  @align(128) @size(128)
  m : ${mat},
  after : i32,
}

struct S_std140 {
  before : i32,
  @align(128i)
${col_vectors}
  after : i32,
}

@group(0) @binding(0) var<uniform> s : S_std140;
)";
        expect = matrix.ReplaceFieldsInString(
            expect,
            {{"${col_vectors}", matrix.ExpendedColumnVectorsWithLastSize(2, "m_", last_size)}});
    } else {
        expect = src;
    }

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_P(Std140Test_Matrix, MatrixUsageInForLoop) {
    auto matrix = GetParam();

    std::string src = R"(
enable f16;

struct S {
  m : ${mat},
}

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  for(var i = u32(s.m[0][0]); (i < u32(s.m[i][1])); i += u32(s.m[1][i])) {
  }
}
)";
    src = matrix.ReplaceFieldsInString(src);

    std::string expect;
    if (matrix.NotStd140Compatible()) {
        expect = R"(
enable f16;

struct S {
  m : ${mat},
}

struct S_std140 {
${col_vectors}
}

@group(0) @binding(0) var<uniform> s : S_std140;

fn load_s_m_p0_1(p0 : u32) -> ${elem_type} {
  switch(p0) {
${col_table}
    default: {
      return ${elem_type}();
    }
  }
}

fn f() {
  for(var i = u32(s.m_0[0u]); (i < u32(load_s_m_p0_1(u32(i)))); i += u32(s.m_1[i])) {
  }
}
)";

        // col_table is the switch cases for all column index.
        // Example for a matrix having 2 columns:
        //   case 0u: {
        //     return s.m_0[1u];
        //   }
        //   case 1u: {
        //     return s.m_1[1u];
        //   }
        std::string col_table = matrix.JoinTemplatedStringForEachMatrixColumn(  //
            R"(    case ${col_id_for_tmpl}u: {
      return s.m_${col_id_for_tmpl}[1u];
    })",
            "\n");
        expect = matrix.ReplaceFieldsInString(
            expect, {{"${col_vectors}", matrix.ExpendedColumnVectors(2, "m_")},
                     {"${col_table}", col_table}});
    } else {
        expect = src;
    }

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_P(Std140Test_Matrix, MatUniform_LoadMatrix) {
    auto matrix = GetParam();

    std::string src = R"(
enable f16;

@group(0) @binding(0) var<uniform> m : ${mat};

fn f() {
  let l = m;
}
)";
    src = matrix.ReplaceFieldsInString(src);

    std::string expect;
    if (matrix.NotStd140Compatible()) {
        expect = R"(
enable f16;

struct mat${shape}_${elem_type} {
${col_vectors}
}

@group(0) @binding(0) var<uniform> m : mat${shape}_${elem_type};

fn conv_mat${shape}_${elem_type}(val : mat${shape}_${elem_type}) -> ${mat} {
  return ${mat}(${col_vectors_inline});
}

fn f() {
  let l = conv_mat${shape}_${elem_type}(m);
}
)";
        expect = matrix.ReplaceFieldsInString(
            expect,
            {{"${col_vectors}", matrix.ExpendedColumnVectors(2, "col")},
             {"${col_vectors_inline}", matrix.ExpendedColumnVectorsInline("val.col", ", ")}});
    } else {
        expect = src;
    }

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_P(Std140Test_Matrix, MatUniform_LoadColumn_ConstIndex) {
    auto matrix = GetParam();

    std::string tmpl_src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : ${mat};

fn f() {
  let l = a[${cloumn_index}];
}
)";
    tmpl_src = matrix.ReplaceFieldsInString(tmpl_src);

    std::string tmpl_expect;
    if (matrix.NotStd140Compatible()) {
        tmpl_expect = R"(
enable f16;

struct mat${shape}_${elem_type} {
${col_vectors}
}

@group(0) @binding(0) var<uniform> a : mat${shape}_${elem_type};

fn f() {
  let l = a.col${cloumn_index};
}
)";
        tmpl_expect = matrix.ReplaceFieldsInString(
            tmpl_expect, {{"${col_vectors}", matrix.ExpendedColumnVectors(2, "col")}});
    } else {
        tmpl_expect = tmpl_src;
    }

    for (uint32_t col = 0; col < matrix.columns; col++) {
        std::string src = utils::ReplaceAll(tmpl_src, "${cloumn_index}", std::to_string(col));
        std::string expect = utils::ReplaceAll(tmpl_expect, "${cloumn_index}", std::to_string(col));

        auto got = Run<Std140>(src);

        EXPECT_EQ(expect, str(got)) << "accessing col " << col;
    }
}

TEST_P(Std140Test_Matrix, MatUniform_LoadColumn_VariableIndex) {
    auto matrix = GetParam();

    std::string src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : ${mat};

fn f() {
  let I = 1;
  let l = a[I];
}
)";
    src = matrix.ReplaceFieldsInString(src);

    std::string expect;
    if (matrix.NotStd140Compatible()) {
        expect = R"(
enable f16;

struct mat${shape}_${elem_type} {
${col_vectors}
}

@group(0) @binding(0) var<uniform> a : mat${shape}_${elem_type};

fn load_a_p0(p0 : u32) -> ${col_vector_type} {
  switch(p0) {
${col_table}
    default: {
      return ${col_vector_type}();
    }
  }
}

fn f() {
  let I = 1;
  let l = load_a_p0(u32(I));
}
)";
        // col_table is the switch cases for all column index.
        // Example for a matrix having 2 columns:
        //   case 0u: {
        //     return a.col0;
        //   }
        //   case 1u: {
        //     return a.col1;
        //   }
        std::string col_table = matrix.JoinTemplatedStringForEachMatrixColumn(  //
            R"(    case ${col_id_for_tmpl}u: {
      return a.col${col_id_for_tmpl};
    })",
            "\n");
        expect = matrix.ReplaceFieldsInString(
            expect, {{"${col_vectors}", matrix.ExpendedColumnVectors(2, "col")},
                     {"${col_table}", col_table}});
    } else {
        expect = src;
    }

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_P(Std140Test_Matrix, MatUniform_LoadColumnSwizzle_ConstIndex) {
    auto matrix = GetParam();

    std::string tmpl_src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : ${mat};

fn f() {
  let l = a[${cloumn_index}].${swizzle};
}
)";
    tmpl_src = matrix.ReplaceFieldsInString(tmpl_src);

    std::string tmpl_expect;
    if (matrix.NotStd140Compatible()) {
        tmpl_expect = R"(
enable f16;

struct mat${shape}_${elem_type} {
${col_vectors}
}

@group(0) @binding(0) var<uniform> a : mat${shape}_${elem_type};

fn f() {
  let l = a.col${cloumn_index}.${swizzle};
}
)";
        tmpl_expect = matrix.ReplaceFieldsInString(
            tmpl_expect, {{"${col_vectors}", matrix.ExpendedColumnVectors(2, "col")}});
    } else {
        tmpl_expect = tmpl_src;
    }

    for (uint32_t col = 0; col < matrix.columns; col++) {
        std::string src = utils::ReplaceAll(tmpl_src, "${cloumn_index}", std::to_string(col));
        std::string expect = utils::ReplaceAll(tmpl_expect, "${cloumn_index}", std::to_string(col));

        auto got = Run<Std140>(src);

        EXPECT_EQ(expect, str(got)) << "accessing col " << col;
    }
}

TEST_P(Std140Test_Matrix, MatUniform_LoadColumnSwizzle_VariableIndex) {
    auto matrix = GetParam();

    std::string src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : ${mat};

fn f() {
  let I = 1;
  let l = a[I].${swizzle};
}
)";
    src = matrix.ReplaceFieldsInString(src);

    std::string expect;
    if (matrix.NotStd140Compatible()) {
        expect = R"(
enable f16;

struct mat${shape}_${elem_type} {
${col_vectors}
}

@group(0) @binding(0) var<uniform> a : mat${shape}_${elem_type};

fn load_a_p0_${swizzle}(p0 : u32) -> ${col_vector_type} {
  switch(p0) {
${col_table}
    default: {
      return ${col_vector_type}();
    }
  }
}

fn f() {
  let I = 1;
  let l = load_a_p0_${swizzle}(u32(I));
}
)";
        // col_table is the switch cases for all column index.
        // Example for a matrix having 2 columns:
        //   case 0u: {
        //     return a.col0.${swizzle};
        //   }
        //   case 1u: {
        //     return a.col1.${swizzle};
        //   }
        std::string col_table = matrix.JoinTemplatedStringForEachMatrixColumn(  //
            R"(    case ${col_id_for_tmpl}u: {
      return a.col${col_id_for_tmpl}.${swizzle};
    })",
            "\n");
        expect = matrix.ReplaceFieldsInString(
            expect, {{"${col_vectors}", matrix.ExpendedColumnVectors(2, "col")},
                     {"${col_table}", col_table}});
    } else {
        expect = src;
    }

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_P(Std140Test_Matrix, MatUniform_LoadScalar_ConstColumnIndex_ConstRowIndex) {
    auto matrix = GetParam();

    std::string tmpl_src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : ${mat};

fn f() {
  let l = a[${col_index}][${row_index}];
}
)";
    tmpl_src = matrix.ReplaceFieldsInString(tmpl_src);

    std::string tmpl_expect;
    if (matrix.NotStd140Compatible()) {
        tmpl_expect = R"(
enable f16;

struct mat${shape}_${elem_type} {
${col_vectors}
}

@group(0) @binding(0) var<uniform> a : mat${shape}_${elem_type};

fn f() {
  let l = a.col${col_index}[${row_index}u];
}
)";
        tmpl_expect = matrix.ReplaceFieldsInString(
            tmpl_expect, {{"${col_vectors}", matrix.ExpendedColumnVectors(2, "col")}});
    } else {
        tmpl_expect = tmpl_src;
    }

    for (uint32_t col = 0; col < matrix.columns; col++) {
        for (uint32_t row = 0; row < matrix.rows; row++) {
            std::string src = utils::ReplaceAll(tmpl_src, "${col_index}", std::to_string(col));
            src = utils::ReplaceAll(src, "${row_index}", std::to_string(row));
            std::string expect =
                utils::ReplaceAll(tmpl_expect, "${col_index}", std::to_string(col));
            expect = utils::ReplaceAll(expect, "${row_index}", std::to_string(row));

            auto got = Run<Std140>(src);

            EXPECT_EQ(expect, str(got)) << "accessing col " << col << " row " << row;
        }
    }
}

TEST_P(Std140Test_Matrix, MatUniform_LoadScalar_VariableColumnIndex_ConstRowIndex) {
    auto matrix = GetParam();

    std::string tmpl_src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : ${mat};

fn f() {
  let I = 0;
  let l = a[I][${row_index}];
}
)";
    tmpl_src = matrix.ReplaceFieldsInString(tmpl_src);

    std::string tmpl_expect;
    if (matrix.NotStd140Compatible()) {
        tmpl_expect = R"(
enable f16;

struct mat${shape}_${elem_type} {
${col_vectors}
}

@group(0) @binding(0) var<uniform> a : mat${shape}_${elem_type};

fn load_a_p0_${row_index}(p0 : u32) -> ${elem_type} {
  switch(p0) {
${col_table}
    default: {
      return ${elem_type}();
    }
  }
}

fn f() {
  let I = 0;
  let l = load_a_p0_${row_index}(u32(I));
}
)";
        // col_table is the switch cases for all column index.
        // Example for a matrix having 2 columns:
        //   case 0u: {
        //     return a.col0[${row_index}u];
        //   }
        //   case 1u: {
        //     return a.col1[${row_index}u];
        //   }
        std::string col_table = matrix.JoinTemplatedStringForEachMatrixColumn(  //
            R"(    case ${col_id_for_tmpl}u: {
      return a.col${col_id_for_tmpl}[${row_index}u];
    })",
            "\n");
        tmpl_expect = matrix.ReplaceFieldsInString(
            tmpl_expect, {{"${col_vectors}", matrix.ExpendedColumnVectors(2, "col")},
                          {"${col_table}", col_table}});
    } else {
        tmpl_expect = tmpl_src;
    }

    for (uint32_t row = 0; row < matrix.rows; row++) {
        std::string src = utils::ReplaceAll(tmpl_src, "${row_index}", std::to_string(row));
        std::string expect = utils::ReplaceAll(tmpl_expect, "${row_index}", std::to_string(row));

        auto got = Run<Std140>(src);

        EXPECT_EQ(expect, str(got)) << "accessing row " << row;
    }
}

TEST_P(Std140Test_Matrix, MatUniform_LoadScalar_ConstColumnIndex_VariableRowIndex) {
    auto matrix = GetParam();

    std::string tmpl_src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : ${mat};

fn f() {
  let I = 0;
  let l = a[${col_index}][I];
}
)";
    tmpl_src = matrix.ReplaceFieldsInString(tmpl_src);

    std::string tmpl_expect;
    if (matrix.NotStd140Compatible()) {
        tmpl_expect = R"(
enable f16;

struct mat${shape}_${elem_type} {
${col_vectors}
}

@group(0) @binding(0) var<uniform> a : mat${shape}_${elem_type};

fn f() {
  let I = 0;
  let l = a.col${col_index}[I];
}
)";
        tmpl_expect = matrix.ReplaceFieldsInString(
            tmpl_expect, {{"${col_vectors}", matrix.ExpendedColumnVectors(2, "col")}});
    } else {
        tmpl_expect = tmpl_src;
    }

    for (uint32_t col = 0; col < matrix.columns; col++) {
        std::string src = utils::ReplaceAll(tmpl_src, "${col_index}", std::to_string(col));
        std::string expect = utils::ReplaceAll(tmpl_expect, "${col_index}", std::to_string(col));

        auto got = Run<Std140>(src);

        EXPECT_EQ(expect, str(got)) << "accessing col " << col;
    }
}

TEST_P(Std140Test_Matrix, MatUniform_LoadScalar_VariableColumnIndex_VariableRowIndex) {
    auto matrix = GetParam();

    std::string src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : ${mat};

fn f() {
  let I = 0;
  let l = a[I][I];
}
)";
    src = matrix.ReplaceFieldsInString(src);

    std::string expect;
    if (matrix.NotStd140Compatible()) {
        expect = R"(
enable f16;

struct mat${shape}_${elem_type} {
${col_vectors}
}

@group(0) @binding(0) var<uniform> a : mat${shape}_${elem_type};

fn load_a_p0_p1(p0 : u32, p1 : u32) -> ${elem_type} {
  switch(p0) {
${col_table}
    default: {
      return ${elem_type}();
    }
  }
}

fn f() {
  let I = 0;
  let l = load_a_p0_p1(u32(I), u32(I));
}
)";

        // col_table is the switch cases for all column index.
        // Example for a matrix having 2 columns:
        //   case 0u: {
        //     return a.col0[p1];
        //   }
        //   case 1u: {
        //     return a.col1[p1];
        //   }
        std::string col_table = matrix.JoinTemplatedStringForEachMatrixColumn(  //
            R"(    case ${col_id_for_tmpl}u: {
      return a.col${col_id_for_tmpl}[p1];
    })",
            "\n");
        expect = matrix.ReplaceFieldsInString(
            expect, {{"${col_vectors}", matrix.ExpendedColumnVectors(2, "col")},
                     {"${col_table}", col_table}});
    } else {
        expect = src;
    }

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_P(Std140Test_Matrix, StructMatUniform_NameCollision) {
    auto matrix = GetParam();

    std::string src = R"(
enable f16;

struct S {
  m_1 : i32,
  m : ${mat},
}

@group(0) @binding(0) var<uniform> s : S;
)";
    src = matrix.ReplaceFieldsInString(src);

    std::string expect;
    if (matrix.NotStd140Compatible()) {
        expect = R"(
enable f16;

struct S {
  m_1 : i32,
  m : ${mat},
}

struct S_std140 {
  m_1 : i32,
${col_vectors}
}

@group(0) @binding(0) var<uniform> s : S_std140;
)";
        expect = matrix.ReplaceFieldsInString(
            expect, {{"${col_vectors}", matrix.ExpendedColumnVectors(2, "m__")}});
    } else {
        expect = src;
    }

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_P(Std140Test_Matrix, StructMatUniform_LoadStruct) {
    auto matrix = GetParam();

    std::string src = R"(
enable f16;

struct S {
  m : ${mat},
}

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  let l = s;
}
)";
    src = matrix.ReplaceFieldsInString(src);

    std::string expect;
    if (matrix.NotStd140Compatible()) {
        expect = R"(
enable f16;

struct S {
  m : ${mat},
}

struct S_std140 {
${col_vectors}
}

@group(0) @binding(0) var<uniform> s : S_std140;

fn conv_S(val : S_std140) -> S {
  return S(${mat}(${col_vectors_inline}));
}

fn f() {
  let l = conv_S(s);
}
)";
        expect = matrix.ReplaceFieldsInString(
            expect,
            {{"${col_vectors}", matrix.ExpendedColumnVectors(2, "m_")},
             {"${col_vectors_inline}", matrix.ExpendedColumnVectorsInline("val.m_", ", ")}});
    } else {
        expect = src;
    }

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_P(Std140Test_Matrix, StructMatUniform_LoadMatrix) {
    auto matrix = GetParam();

    std::string src = R"(
enable f16;

struct S {
  m : ${mat},
}

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  let l = s.m;
}
)";
    src = matrix.ReplaceFieldsInString(src);

    std::string expect;
    if (matrix.NotStd140Compatible()) {
        expect = R"(
enable f16;

struct S {
  m : ${mat},
}

struct S_std140 {
${col_vectors}
}

@group(0) @binding(0) var<uniform> s : S_std140;

fn load_s_m() -> ${mat} {
  let s = &(s);
  return ${mat}(${col_vectors_inline});
}

fn f() {
  let l = load_s_m();
}
)";
        expect = matrix.ReplaceFieldsInString(
            expect,
            {{"${col_vectors}", matrix.ExpendedColumnVectors(2, "m_")},
             {"${col_vectors_inline}", matrix.ExpendedColumnVectorsInline("(*(s)).m_", ", ")}});
    } else {
        expect = src;
    }

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_P(Std140Test_Matrix, StructMatUniform_LoadColumn_ConstIndex) {
    auto matrix = GetParam();

    std::string tmpl_src = R"(
enable f16;

struct S {
  m : ${mat},
}

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  let l = s.m[${col_index}];
}
)";
    tmpl_src = matrix.ReplaceFieldsInString(tmpl_src);

    std::string tmpl_expect;
    if (matrix.NotStd140Compatible()) {
        tmpl_expect = R"(
enable f16;

struct S {
  m : ${mat},
}

struct S_std140 {
${col_vectors}
}

@group(0) @binding(0) var<uniform> s : S_std140;

fn f() {
  let l = s.m_${col_index};
}
)";
        tmpl_expect = matrix.ReplaceFieldsInString(
            tmpl_expect, {{"${col_vectors}", matrix.ExpendedColumnVectors(2, "m_")}});
    } else {
        tmpl_expect = tmpl_src;
    }

    for (uint32_t col = 0; col < matrix.columns; col++) {
        std::string src = utils::ReplaceAll(tmpl_src, "${col_index}", std::to_string(col));
        std::string expect = utils::ReplaceAll(tmpl_expect, "${col_index}", std::to_string(col));

        auto got = Run<Std140>(src);

        EXPECT_EQ(expect, str(got)) << "accessing col " << col;
    }
}

TEST_P(Std140Test_Matrix, StructMatUniform_LoadColumn_VariableIndex) {
    auto matrix = GetParam();

    std::string src = R"(
enable f16;

struct S {
  m : ${mat},
}

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  let I = 0;
  let l = s.m[I];
}
)";
    src = matrix.ReplaceFieldsInString(src);

    std::string expect;
    if (matrix.NotStd140Compatible()) {
        expect = R"(
enable f16;

struct S {
  m : ${mat},
}

struct S_std140 {
${col_vectors}
}

@group(0) @binding(0) var<uniform> s : S_std140;

fn load_s_m_p0(p0 : u32) -> ${col_vector_type} {
  switch(p0) {
${col_table}
    default: {
      return ${col_vector_type}();
    }
  }
}

fn f() {
  let I = 0;
  let l = load_s_m_p0(u32(I));
}
)";

        // col_table is the switch cases for all column index.
        // Example for a matrix having 2 columns:
        //   case 0u: {
        //     return s.m_0;
        //   }
        //   case 1u: {
        //     return s.m_1;
        //   }
        std::string col_table = matrix.JoinTemplatedStringForEachMatrixColumn(  //
            R"(    case ${col_id_for_tmpl}u: {
      return s.m_${col_id_for_tmpl};
    })",
            "\n");
        expect = matrix.ReplaceFieldsInString(
            expect, {{"${col_vector_type}", matrix.ColumnVector()},
                     {"${col_vectors}", matrix.ExpendedColumnVectors(2, "m_")},
                     {"${col_table}", col_table}});
    } else {
        expect = src;
    }

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_P(Std140Test_Matrix, StructMatUniform_LoadScalar_ConstColumnIndex_ConstRowIndex) {
    auto matrix = GetParam();

    std::string tmpl_src = R"(
enable f16;

struct S {
  m : ${mat},
}

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  let l = s.m[${col_index}][${row_index}];
}
)";
    tmpl_src = matrix.ReplaceFieldsInString(tmpl_src);

    std::string tmpl_expect;
    if (matrix.NotStd140Compatible()) {
        tmpl_expect = R"(
enable f16;

struct S {
  m : ${mat},
}

struct S_std140 {
${col_vectors}
}

@group(0) @binding(0) var<uniform> s : S_std140;

fn f() {
  let l = s.m_${col_index}[${row_index}u];
}
)";
        tmpl_expect = matrix.ReplaceFieldsInString(
            tmpl_expect, {{"${col_vectors}", matrix.ExpendedColumnVectors(2, "m_")}});
    } else {
        tmpl_expect = tmpl_src;
    }

    for (uint32_t col = 0; col < matrix.columns; col++) {
        for (uint32_t row = 0; row < matrix.rows; row++) {
            std::string src = utils::ReplaceAll(tmpl_src, "${col_index}", std::to_string(col));
            src = utils::ReplaceAll(src, "${row_index}", std::to_string(row));
            std::string expect =
                utils::ReplaceAll(tmpl_expect, "${col_index}", std::to_string(col));
            expect = utils::ReplaceAll(expect, "${row_index}", std::to_string(row));

            auto got = Run<Std140>(src);

            EXPECT_EQ(expect, str(got)) << "accessing col " << col << " row " << row;
        }
    }
}

TEST_P(Std140Test_Matrix, StructMatUniform_LoadScalar_VariableColumnIndex_ConstRowIndex) {
    auto matrix = GetParam();

    std::string tmpl_src = R"(
enable f16;

struct S {
  m : ${mat},
}

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  let I = 0;
  let l = s.m[I][${row_index}];
}
)";
    tmpl_src = matrix.ReplaceFieldsInString(tmpl_src);

    std::string tmpl_expect;
    if (matrix.NotStd140Compatible()) {
        tmpl_expect = R"(
enable f16;

struct S {
  m : ${mat},
}

struct S_std140 {
${col_vectors}
}

@group(0) @binding(0) var<uniform> s : S_std140;

fn load_s_m_p0_${row_index}(p0 : u32) -> ${elem_type} {
  switch(p0) {
${col_table}
    default: {
      return ${elem_type}();
    }
  }
}

fn f() {
  let I = 0;
  let l = load_s_m_p0_${row_index}(u32(I));
}
)";
        // col_table is the switch cases for all column index.
        // Example for a matrix having 2 columns:
        //   case 0u: {
        //     return s.m_0[${row_index}u];
        //   }
        //   case 1u: {
        //     return s.m_1[${row_index}u];
        //   }
        std::string col_table = matrix.JoinTemplatedStringForEachMatrixColumn(  //
            R"(    case ${col_id_for_tmpl}u: {
      return s.m_${col_id_for_tmpl}[${row_index}u];
    })",
            "\n");
        tmpl_expect = matrix.ReplaceFieldsInString(
            tmpl_expect, {{"${col_vectors}", matrix.ExpendedColumnVectors(2, "m_")},
                          {"${col_table}", col_table}});
    } else {
        tmpl_expect = tmpl_src;
    }

    for (uint32_t row = 0; row < matrix.rows; row++) {
        std::string src = utils::ReplaceAll(tmpl_src, "${row_index}", std::to_string(row));
        std::string expect = utils::ReplaceAll(tmpl_expect, "${row_index}", std::to_string(row));

        auto got = Run<Std140>(src);

        EXPECT_EQ(expect, str(got)) << "accessing row " << row;
    }
}

TEST_P(Std140Test_Matrix, StructMatUniform_LoadScalar_ConstColumnIndex_VariableRowIndex) {
    auto matrix = GetParam();

    std::string tmpl_src = R"(
enable f16;

struct S {
  m : ${mat},
}

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  let I = 0;
  let l = s.m[${col_index}][I];
}
)";
    tmpl_src = matrix.ReplaceFieldsInString(tmpl_src);

    std::string tmpl_expect;
    if (matrix.NotStd140Compatible()) {
        tmpl_expect = R"(
enable f16;

struct S {
  m : ${mat},
}

struct S_std140 {
${col_vectors}
}

@group(0) @binding(0) var<uniform> s : S_std140;

fn f() {
  let I = 0;
  let l = s.m_${col_index}[I];
}
)";
        tmpl_expect = matrix.ReplaceFieldsInString(
            tmpl_expect, {{"${col_vectors}", matrix.ExpendedColumnVectors(2, "m_")}});
    } else {
        tmpl_expect = tmpl_src;
    }

    for (uint32_t col = 0; col < matrix.columns; col++) {
        std::string src = utils::ReplaceAll(tmpl_src, "${col_index}", std::to_string(col));
        std::string expect = utils::ReplaceAll(tmpl_expect, "${col_index}", std::to_string(col));

        auto got = Run<Std140>(src);

        EXPECT_EQ(expect, str(got)) << "accessing col " << col;
    }
}

TEST_P(Std140Test_Matrix, StructMatUniform_LoadScalar_VariableColumnIndex_VariableRowIndex) {
    auto matrix = GetParam();

    std::string src = R"(
enable f16;

struct S {
  m : ${mat},
}

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  let I = 0;
  let l = s.m[I][I];
}
)";
    src = matrix.ReplaceFieldsInString(src);

    std::string expect;
    if (matrix.NotStd140Compatible()) {
        expect = R"(
enable f16;

struct S {
  m : ${mat},
}

struct S_std140 {
${col_vectors}
}

@group(0) @binding(0) var<uniform> s : S_std140;

fn load_s_m_p0_p1(p0 : u32, p1 : u32) -> ${elem_type} {
  switch(p0) {
${col_table}
    default: {
      return ${elem_type}();
    }
  }
}

fn f() {
  let I = 0;
  let l = load_s_m_p0_p1(u32(I), u32(I));
}
)";

        // col_table is the switch cases for all column index.
        // Example for a matrix having 2 columns:
        //   case 0u: {
        //     return s.m_0[p1];
        //   }
        //   case 1u: {
        //     return s.m_1[p1];
        //   }
        std::string col_table = matrix.JoinTemplatedStringForEachMatrixColumn(  //
            R"(    case ${col_id_for_tmpl}u: {
      return s.m_${col_id_for_tmpl}[p1];
    })",
            "\n");
        expect = matrix.ReplaceFieldsInString(
            expect, {{"${col_vectors}", matrix.ExpendedColumnVectors(2, "m_")},
                     {"${col_table}", col_table}});
    } else {
        expect = src;
    }

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_P(Std140Test_Matrix, ArrayStructMatUniform_LoadArray) {
    auto matrix = GetParam();

    std::string src = R"(
enable f16;

struct S {
  @size(64)
  m : ${mat},
}

@group(0) @binding(0) var<uniform> a : array<S, 3>;

fn f() {
  let l = a;
}
)";
    src = matrix.ReplaceFieldsInString(src);

    std::string expect;
    if (matrix.NotStd140Compatible()) {
        expect = R"(
enable f16;

struct S {
  @size(64)
  m : ${mat},
}

struct S_std140 {
${col_vectors}
}

@group(0) @binding(0) var<uniform> a : array<S_std140, 3u>;

fn conv_S(val : S_std140) -> S {
  return S(${mat}(${col_vectors_inline}));
}

fn conv_arr3_S(val : array<S_std140, 3u>) -> array<S, 3u> {
  var arr : array<S, 3u>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    arr[i] = conv_S(val[i]);
  }
  return arr;
}

fn f() {
  let l = conv_arr3_S(a);
}
)";
        uint32_t last_size =
            64 - static_cast<uint32_t>(matrix.ColumnVectorAlign() * (matrix.columns - 1));
        expect = matrix.ReplaceFieldsInString(
            expect,
            {{"${col_vectors}", matrix.ExpendedColumnVectorsWithLastSize(2, "m_", last_size)},
             {"${col_vectors_inline}", matrix.ExpendedColumnVectorsInline("val.m_", ", ")}});
    } else {
        expect = src;
    }

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_P(Std140Test_Matrix, ArrayStructMatUniform_LoadStruct_ConstIndex) {
    auto matrix = GetParam();

    std::string tmpl_src = R"(
enable f16;

struct S {
  @size(64)
  m : ${mat},
}

@group(0) @binding(0) var<uniform> a : array<S, 3>;

fn f() {
  let l = a[${array_index}];
}
)";
    tmpl_src = matrix.ReplaceFieldsInString(tmpl_src);

    std::string tmpl_expect;
    if (matrix.NotStd140Compatible()) {
        tmpl_expect = R"(
enable f16;

struct S {
  @size(64)
  m : ${mat},
}

struct S_std140 {
${col_vectors}
}

@group(0) @binding(0) var<uniform> a : array<S_std140, 3u>;

fn conv_S(val : S_std140) -> S {
  return S(${mat}(${col_vectors_inline}));
}

fn f() {
  let l = conv_S(a[${array_index}u]);
}
)";
        uint32_t last_size =
            64 - static_cast<uint32_t>(matrix.ColumnVectorAlign() * (matrix.columns - 1));
        tmpl_expect = matrix.ReplaceFieldsInString(
            tmpl_expect,
            {{"${col_vectors}", matrix.ExpendedColumnVectorsWithLastSize(2, "m_", last_size)},
             {"${col_vectors_inline}", matrix.ExpendedColumnVectorsInline("val.m_", ", ")}});
    } else {
        tmpl_expect = tmpl_src;
    }

    for (uint32_t array_index = 0; array_index < 3; array_index++) {
        std::string src =
            utils::ReplaceAll(tmpl_src, "${array_index}", std::to_string(array_index));
        std::string expect =
            utils::ReplaceAll(tmpl_expect, "${array_index}", std::to_string(array_index));

        auto got = Run<Std140>(src);

        EXPECT_EQ(expect, str(got)) << "accessing array element " << array_index;
    }
}

TEST_P(Std140Test_Matrix, ArrayStructMatUniform_LoadStruct_VariableIndex) {
    auto matrix = GetParam();

    std::string src = R"(
enable f16;

struct S {
  @size(64)
  m : ${mat},
}

@group(0) @binding(0) var<uniform> a : array<S, 3>;

fn f() {
  let I = 1;
  let l = a[I];
}
)";
    src = matrix.ReplaceFieldsInString(src);

    std::string expect;
    if (matrix.NotStd140Compatible()) {
        expect = R"(
enable f16;

struct S {
  @size(64)
  m : ${mat},
}

struct S_std140 {
${col_vectors}
}

@group(0) @binding(0) var<uniform> a : array<S_std140, 3u>;

fn conv_S(val : S_std140) -> S {
  return S(${mat}(${col_vectors_inline}));
}

fn f() {
  let I = 1;
  let l = conv_S(a[I]);
}
)";
        uint32_t last_size =
            64 - static_cast<uint32_t>(matrix.ColumnVectorAlign() * (matrix.columns - 1));
        expect = matrix.ReplaceFieldsInString(
            expect,
            {{"${col_vectors}", matrix.ExpendedColumnVectorsWithLastSize(2, "m_", last_size)},
             {"${col_vectors_inline}", matrix.ExpendedColumnVectorsInline("val.m_", ", ")}});
    } else {
        expect = src;
    }

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_P(Std140Test_Matrix, ArrayStructMatUniform_LoadMatrix_ConstArrayIndex) {
    auto matrix = GetParam();

    std::string tmpl_src = R"(
enable f16;

struct S {
  @size(64)
  m : ${mat},
}

@group(0) @binding(0) var<uniform> a : array<S, 3>;

fn f() {
  let l = a[${array_index}].m;
}
)";
    tmpl_src = matrix.ReplaceFieldsInString(tmpl_src);

    std::string tmpl_expect;
    if (matrix.NotStd140Compatible()) {
        tmpl_expect = R"(
enable f16;

struct S {
  @size(64)
  m : ${mat},
}

struct S_std140 {
${col_vectors}
}

@group(0) @binding(0) var<uniform> a : array<S_std140, 3u>;

fn load_a_${array_index}_m() -> ${mat} {
  let s = &(a[${array_index}u]);
  return ${mat}(${col_vectors_inline});
}

fn f() {
  let l = load_a_${array_index}_m();
}
)";
        uint32_t last_size =
            64 - static_cast<uint32_t>(matrix.ColumnVectorAlign() * (matrix.columns - 1));
        tmpl_expect = matrix.ReplaceFieldsInString(
            tmpl_expect,
            {{"${col_vectors}", matrix.ExpendedColumnVectorsWithLastSize(2, "m_", last_size)},
             {"${col_vectors_inline}", matrix.ExpendedColumnVectorsInline("(*(s)).m_", ", ")}});
    } else {
        tmpl_expect = tmpl_src;
    }

    for (uint32_t array_index = 0; array_index < 3; array_index++) {
        std::string src =
            utils::ReplaceAll(tmpl_src, "${array_index}", std::to_string(array_index));
        std::string expect =
            utils::ReplaceAll(tmpl_expect, "${array_index}", std::to_string(array_index));

        auto got = Run<Std140>(src);

        EXPECT_EQ(expect, str(got)) << "accessing array element " << array_index;
    }
}

TEST_P(Std140Test_Matrix, ArrayStructMatUniform_LoadMatrix_VariableArrayIndex) {
    auto matrix = GetParam();

    std::string src = R"(
enable f16;

struct S {
  @size(64)
  m : ${mat},
}

@group(0) @binding(0) var<uniform> a : array<S, 3>;

fn f() {
  let I = 1;
  let l = a[I].m;
}
)";
    src = matrix.ReplaceFieldsInString(src);

    std::string expect;
    if (matrix.NotStd140Compatible()) {
        expect = R"(
enable f16;

struct S {
  @size(64)
  m : ${mat},
}

struct S_std140 {
${col_vectors}
}

@group(0) @binding(0) var<uniform> a : array<S_std140, 3u>;

fn load_a_p0_m(p0 : u32) -> ${mat} {
  let s = &(a[p0]);
  return ${mat}(${col_vectors_inline});
}

fn f() {
  let I = 1;
  let l = load_a_p0_m(u32(I));
}
)";
        uint32_t last_size =
            64 - static_cast<uint32_t>(matrix.ColumnVectorAlign() * (matrix.columns - 1));
        expect = matrix.ReplaceFieldsInString(
            expect,
            {{"${col_vectors}", matrix.ExpendedColumnVectorsWithLastSize(2, "m_", last_size)},
             {"${col_vectors_inline}", matrix.ExpendedColumnVectorsInline("(*(s)).m_", ", ")}});
    } else {
        expect = src;
    }

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_P(Std140Test_Matrix, ArrayStructMatUniform_LoadColumn_ConstArrayIndex_ConstColumnIndex) {
    auto matrix = GetParam();

    std::string tmpl_src = R"(
enable f16;

struct S {
  @size(64)
  m : ${mat},
}

@group(0) @binding(0) var<uniform> a : array<S, 3>;

fn f() {
  let l = a[${array_index}].m[${cloumn_index}];
}
)";
    tmpl_src = matrix.ReplaceFieldsInString(tmpl_src);

    std::string tmpl_expect;
    if (matrix.NotStd140Compatible()) {
        tmpl_expect = R"(
enable f16;

struct S {
  @size(64)
  m : ${mat},
}

struct S_std140 {
${col_vectors}
}

@group(0) @binding(0) var<uniform> a : array<S_std140, 3u>;

fn f() {
  let l = a[${array_index}u].m_${cloumn_index};
}
)";
        uint32_t last_size =
            64 - static_cast<uint32_t>(matrix.ColumnVectorAlign() * (matrix.columns - 1));
        tmpl_expect = matrix.ReplaceFieldsInString(
            tmpl_expect,
            {{"${col_vectors}", matrix.ExpendedColumnVectorsWithLastSize(2, "m_", last_size)}});
    } else {
        tmpl_expect = tmpl_src;
    }

    for (uint32_t array_index = 0; array_index < 3; array_index++) {
        for (uint32_t col = 0; col < matrix.columns; col++) {
            std::string src =
                utils::ReplaceAll(tmpl_src, "${array_index}", std::to_string(array_index));
            src = utils::ReplaceAll(src, "${cloumn_index}", std::to_string(col));
            std::string expect =
                utils::ReplaceAll(tmpl_expect, "${array_index}", std::to_string(array_index));
            expect = utils::ReplaceAll(expect, "${cloumn_index}", std::to_string(col));

            auto got = Run<Std140>(src);

            EXPECT_EQ(expect, str(got))
                << "accessing array element " << array_index << " col " << col;
        }
    }
}

TEST_P(Std140Test_Matrix, ArrayStructMatUniform_LoadColumn_VariableArrayIndex_ConstColumnIndex) {
    auto matrix = GetParam();

    std::string tmpl_src = R"(
enable f16;

struct S {
  @size(64)
  m : ${mat},
}

@group(0) @binding(0) var<uniform> a : array<S, 3>;

fn f() {
  let I = 1;
  let l = a[I].m[${cloumn_index}];
}
)";
    tmpl_src = matrix.ReplaceFieldsInString(tmpl_src);

    std::string tmpl_expect;
    if (matrix.NotStd140Compatible()) {
        tmpl_expect = R"(
enable f16;

struct S {
  @size(64)
  m : ${mat},
}

struct S_std140 {
${col_vectors}
}

@group(0) @binding(0) var<uniform> a : array<S_std140, 3u>;

fn f() {
  let I = 1;
  let l = a[I].m_${cloumn_index};
}
)";
        uint32_t last_size =
            64 - static_cast<uint32_t>(matrix.ColumnVectorAlign() * (matrix.columns - 1));
        tmpl_expect = matrix.ReplaceFieldsInString(
            tmpl_expect,
            {{"${col_vectors}", matrix.ExpendedColumnVectorsWithLastSize(2, "m_", last_size)}});
    } else {
        tmpl_expect = tmpl_src;
    }

    for (uint32_t col = 0; col < matrix.columns; col++) {
        std::string src = utils::ReplaceAll(tmpl_src, "${cloumn_index}", std::to_string(col));
        std::string expect = utils::ReplaceAll(tmpl_expect, "${cloumn_index}", std::to_string(col));

        auto got = Run<Std140>(src);

        EXPECT_EQ(expect, str(got)) << "accessing col " << col;
    }
}

TEST_P(Std140Test_Matrix, ArrayStructMatUniform_LoadColumn_ConstArrayIndex_VariableColumnIndex) {
    auto matrix = GetParam();

    std::string tmpl_src = R"(
enable f16;

struct S {
  @size(64)
  m : ${mat},
}

@group(0) @binding(0) var<uniform> a : array<S, 3>;

fn f() {
  let I = 1;
  let l = a[${array_index}].m[I];
}
)";
    tmpl_src = matrix.ReplaceFieldsInString(tmpl_src);

    std::string tmpl_expect;
    if (matrix.NotStd140Compatible()) {
        tmpl_expect = R"(
enable f16;

struct S {
  @size(64)
  m : ${mat},
}

struct S_std140 {
${col_vectors}
}

@group(0) @binding(0) var<uniform> a : array<S_std140, 3u>;

fn load_a_${array_index}_m_p0(p0 : u32) -> ${col_vector_type} {
  switch(p0) {
${col_table}
    default: {
      return ${col_vector_type}();
    }
  }
}

fn f() {
  let I = 1;
  let l = load_a_${array_index}_m_p0(u32(I));
}
)";
        // col_table is the switch cases for all column index.
        // Example for a matrix having 2 columns:
        //   case 0u: {
        //     return a[${array_index}u].m_0;
        //   }
        //   case 1u: {
        //     return a[${array_index}u].m_1;
        //   }
        std::string col_table = matrix.JoinTemplatedStringForEachMatrixColumn(  //
            R"(    case ${col_id_for_tmpl}u: {
      return a[${array_index}u].m_${col_id_for_tmpl};
    })",
            "\n");
        uint32_t last_size =
            64 - static_cast<uint32_t>(matrix.ColumnVectorAlign() * (matrix.columns - 1));
        tmpl_expect = matrix.ReplaceFieldsInString(
            tmpl_expect,
            {{"${col_vectors}", matrix.ExpendedColumnVectorsWithLastSize(2, "m_", last_size)},
             {"${col_table}", col_table}});
    } else {
        tmpl_expect = tmpl_src;
    }

    for (uint32_t array_index = 0; array_index < 3; array_index++) {
        std::string src =
            utils::ReplaceAll(tmpl_src, "${array_index}", std::to_string(array_index));
        std::string expect =
            utils::ReplaceAll(tmpl_expect, "${array_index}", std::to_string(array_index));

        auto got = Run<Std140>(src);

        EXPECT_EQ(expect, str(got)) << "accessing array element " << array_index;
    }
}

TEST_P(Std140Test_Matrix, ArrayStructMatUniform_LoadColumn_VariableArrayIndex_VariableColumnIndex) {
    auto matrix = GetParam();

    std::string src = R"(
enable f16;

struct S {
  @size(64)
  m : ${mat},
}

@group(0) @binding(0) var<uniform> a : array<S, 3>;

fn f() {
  let I = 1;
  let l = a[I].m[I];
}
)";
    src = matrix.ReplaceFieldsInString(src);

    std::string expect;
    if (matrix.NotStd140Compatible()) {
        expect = R"(
enable f16;

struct S {
  @size(64)
  m : ${mat},
}

struct S_std140 {
${col_vectors}
}

@group(0) @binding(0) var<uniform> a : array<S_std140, 3u>;

fn load_a_p0_m_p1(p0 : u32, p1 : u32) -> ${col_vector_type} {
  switch(p1) {
${col_table}
    default: {
      return ${col_vector_type}();
    }
  }
}

fn f() {
  let I = 1;
  let l = load_a_p0_m_p1(u32(I), u32(I));
}
)";
        // col_table is the switch cases for all column index.
        // Example for a matrix having 2 columns:
        //   case 0u: {
        //     return a[p0].m_0;
        //   }
        //   case 1u: {
        //     return a[p0].m_1;
        //   }
        std::string col_table = matrix.JoinTemplatedStringForEachMatrixColumn(  //
            R"(    case ${col_id_for_tmpl}u: {
      return a[p0].m_${col_id_for_tmpl};
    })",
            "\n");
        uint32_t last_size =
            64 - static_cast<uint32_t>(matrix.ColumnVectorAlign() * (matrix.columns - 1));
        expect = matrix.ReplaceFieldsInString(
            expect,
            {{"${col_vectors}", matrix.ExpendedColumnVectorsWithLastSize(2, "m_", last_size)},
             {"${col_table}", col_table}});
    } else {
        expect = src;
    }

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_P(Std140Test_Matrix, ArrayStructArrayStructMatUniform_Loads) {
    auto matrix = GetParam();

    std::string src = R"(
enable f16;

struct Inner {
  @size(64)
  m : ${mat},
}

struct Outer {
  a : array<Inner, 4>,
}

@group(0) @binding(0) var<uniform> a : array<Outer, 4>;

fn f() {
  let I = 1;
  let J = 2;
  let K = 0;
  let l_a : array<Outer, 4> = a;
  let l_a_1 : Outer = a[1];
  let l_a_I : Outer = a[I];
  let l_a_2_a : array<Inner, 4> = a[2].a;
  let l_a_I_a : array<Inner, 4> = a[I].a;
  let l_a_3_a_1 : Inner = a[3].a[1];
  let l_a_3_a_I : Inner = a[3].a[I];
  let l_a_I_a_1 : Inner = a[I].a[1];
  let l_a_I_a_J : Inner = a[I].a[J];
  let l_a_0_a_2_m : ${mat} = a[0].a[2].m;
  let l_a_0_a_I_m : ${mat} = a[0].a[I].m;
  let l_a_I_a_2_m : ${mat} = a[I].a[2].m;
  let l_a_I_a_J_m : ${mat} = a[I].a[J].m;
  let l_a_1_a_3_m_0 : ${col_vector_type} = a[1].a[3].m[0];
  let l_a_I_a_J_m_K : ${col_vector_type} = a[I].a[J].m[K];
  let l_a_2_a_0_m_1_0 : ${elem_type} = a[2].a[0].m[1][0];
  let l_a_I_a_J_m_K_I : ${elem_type} = a[I].a[J].m[K][I];
}
)";
    src = matrix.ReplaceFieldsInString(src);

    std::string expect;
    if (matrix.NotStd140Compatible()) {
        expect = R"(
enable f16;

struct Inner {
  @size(64)
  m : ${mat},
}

struct Inner_std140 {
${col_vectors}
}

struct Outer {
  a : array<Inner, 4>,
}

struct Outer_std140 {
  a : array<Inner_std140, 4u>,
}

@group(0) @binding(0) var<uniform> a : array<Outer_std140, 4u>;

fn conv_Inner(val : Inner_std140) -> Inner {
  return Inner(${mat}(${col_vectors_inline_conv_Inner}));
}

fn conv_arr4_Inner(val : array<Inner_std140, 4u>) -> array<Inner, 4u> {
  var arr : array<Inner, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    arr[i] = conv_Inner(val[i]);
  }
  return arr;
}

fn conv_Outer(val : Outer_std140) -> Outer {
  return Outer(conv_arr4_Inner(val.a));
}

fn conv_arr4_Outer(val : array<Outer_std140, 4u>) -> array<Outer, 4u> {
  var arr : array<Outer, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    arr[i] = conv_Outer(val[i]);
  }
  return arr;
}

fn load_a_0_a_2_m() -> ${mat} {
  let s = &(a[0u].a[2u]);
  return ${mat}(${col_vectors_inline_load_matrix});
}

fn load_a_0_a_p0_m(p0 : u32) -> ${mat} {
  let s = &(a[0u].a[p0]);
  return ${mat}(${col_vectors_inline_load_matrix});
}

fn load_a_p0_a_2_m(p0 : u32) -> ${mat} {
  let s = &(a[p0].a[2u]);
  return ${mat}(${col_vectors_inline_load_matrix});
}

fn load_a_p0_a_p1_m(p0 : u32, p1 : u32) -> ${mat} {
  let s = &(a[p0].a[p1]);
  return ${mat}(${col_vectors_inline_load_matrix});
}

fn load_a_p0_a_p1_m_p2(p0 : u32, p1 : u32, p2 : u32) -> ${col_vector_type} {
  switch(p2) {
${col_table_load_column}
    default: {
      return ${col_vector_type}();
    }
  }
}

fn load_a_p0_a_p1_m_p2_p3(p0 : u32, p1 : u32, p2 : u32, p3 : u32) -> ${elem_type} {
  switch(p2) {
${col_table_load_element}
    default: {
      return ${elem_type}();
    }
  }
}

fn f() {
  let I = 1;
  let J = 2;
  let K = 0;
  let l_a : array<Outer, 4> = conv_arr4_Outer(a);
  let l_a_1 : Outer = conv_Outer(a[1u]);
  let l_a_I : Outer = conv_Outer(a[I]);
  let l_a_2_a : array<Inner, 4> = conv_arr4_Inner(a[2u].a);
  let l_a_I_a : array<Inner, 4> = conv_arr4_Inner(a[I].a);
  let l_a_3_a_1 : Inner = conv_Inner(a[3u].a[1u]);
  let l_a_3_a_I : Inner = conv_Inner(a[3u].a[I]);
  let l_a_I_a_1 : Inner = conv_Inner(a[I].a[1u]);
  let l_a_I_a_J : Inner = conv_Inner(a[I].a[J]);
  let l_a_0_a_2_m : ${mat} = load_a_0_a_2_m();
  let l_a_0_a_I_m : ${mat} = load_a_0_a_p0_m(u32(I));
  let l_a_I_a_2_m : ${mat} = load_a_p0_a_2_m(u32(I));
  let l_a_I_a_J_m : ${mat} = load_a_p0_a_p1_m(u32(I), u32(J));
  let l_a_1_a_3_m_0 : ${col_vector_type} = a[1u].a[3u].m_0;
  let l_a_I_a_J_m_K : ${col_vector_type} = load_a_p0_a_p1_m_p2(u32(I), u32(J), u32(K));
  let l_a_2_a_0_m_1_0 : ${elem_type} = a[2u].a[0u].m_1[0u];
  let l_a_I_a_J_m_K_I : ${elem_type} = load_a_p0_a_p1_m_p2_p3(u32(I), u32(J), u32(K), u32(I));
}
)";
        std::string col_tableLoadColumn = matrix.JoinTemplatedStringForEachMatrixColumn(  //
            R"(    case ${col_id_for_tmpl}u: {
      return a[p0].a[p1].m_${col_id_for_tmpl};
    })",
            "\n");
        std::string col_tableLoadElement = matrix.JoinTemplatedStringForEachMatrixColumn(  //
            R"(    case ${col_id_for_tmpl}u: {
      return a[p0].a[p1].m_${col_id_for_tmpl}[p3];
    })",
            "\n");
        uint32_t last_size =
            64 - static_cast<uint32_t>(matrix.ColumnVectorAlign() * (matrix.columns - 1));
        expect = matrix.ReplaceFieldsInString(
            expect,
            {{"${col_vectors}", matrix.ExpendedColumnVectorsWithLastSize(2, "m_", last_size)},
             {"${col_vectors_inline_conv_Inner}",
              matrix.ExpendedColumnVectorsInline("val.m_", ", ")},
             {"${col_vectors_inline_load_matrix}",
              matrix.ExpendedColumnVectorsInline("(*(s)).m_", ", ")},
             {"${col_table_load_column}", col_tableLoadColumn},
             {"${col_table_load_element}", col_tableLoadElement}});
    } else {
        expect = src;
    }

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_P(Std140Test_Matrix, ArrayStructArrayStructMatUniform_LoadsViaPtrs) {
    auto matrix = GetParam();

    std::string src = R"(
enable f16;

struct Inner {
  @size(64)
  m : ${mat},
}

struct Outer {
  a : array<Inner, 4>,
}

@group(0) @binding(0) var<uniform> a : array<Outer, 4>;

fn f() {
  let I = 1;
  let J = 2;
  let K = 0;
  let p_a = &(a);
  let p_a_3 = &((*(p_a))[3]);
  let p_a_I = &((*(p_a))[I]);
  let p_a_3_a = &((*(p_a_3)).a);
  let p_a_I_a = &((*(p_a_I)).a);
  let p_a_3_a_2 = &((*(p_a_3_a))[2]);
  let p_a_3_a_I = &((*(p_a_3_a))[I]);
  let p_a_I_a_2 = &((*(p_a_I_a))[2]);
  let p_a_I_a_J = &((*(p_a_I_a))[J]);
  let p_a_3_a_2_m = &((*(p_a_3_a_2)).m);
  let p_a_3_a_I_m = &((*(p_a_3_a_I)).m);
  let p_a_I_a_2_m = &((*(p_a_I_a_2)).m);
  let p_a_I_a_J_m = &((*(p_a_I_a_J)).m);
  let p_a_3_a_2_m_1 = &((*(p_a_3_a_2_m))[1]);
  let p_a_I_a_J_m_K = &((*(p_a_I_a_J_m))[K]);
  let l_a : array<Outer, 4> = *(p_a);
  let l_a_3 : Outer = *(p_a_3);
  let l_a_I : Outer = *(p_a_I);
  let l_a_3_a : array<Inner, 4> = *(p_a_3_a);
  let l_a_I_a : array<Inner, 4> = *(p_a_I_a);
  let l_a_3_a_2 : Inner = *(p_a_3_a_2);
  let l_a_3_a_I : Inner = *(p_a_3_a_I);
  let l_a_I_a_2 : Inner = *(p_a_I_a_2);
  let l_a_I_a_J : Inner = *(p_a_I_a_J);
  let l_a_3_a_2_m : ${mat} = *(p_a_3_a_2_m);
  let l_a_3_a_I_m : ${mat} = *(p_a_3_a_I_m);
  let l_a_I_a_2_m : ${mat} = *(p_a_I_a_2_m);
  let l_a_I_a_J_m : ${mat} = *(p_a_I_a_J_m);
  let l_a_3_a_2_m_1 : ${col_vector_type} = *(p_a_3_a_2_m_1);
  let l_a_I_a_J_m_K : ${col_vector_type} = *(p_a_I_a_J_m_K);
  let l_a_2_a_0_m_1_0 : ${elem_type} = (*(p_a_3_a_2_m_1))[0];
  let l_a_I_a_J_m_K_I : ${elem_type} = (*(p_a_I_a_J_m_K))[I];
}
)";
    src = matrix.ReplaceFieldsInString(src);

    std::string expect;
    if (matrix.NotStd140Compatible()) {
        expect = R"(
enable f16;

struct Inner {
  @size(64)
  m : ${mat},
}

struct Inner_std140 {
${col_vectors}
}

struct Outer {
  a : array<Inner, 4>,
}

struct Outer_std140 {
  a : array<Inner_std140, 4u>,
}

@group(0) @binding(0) var<uniform> a : array<Outer_std140, 4u>;

fn conv_Inner(val : Inner_std140) -> Inner {
  return Inner(${mat}(${col_vectors_inline_conv_Inner}));
}

fn conv_arr4_Inner(val : array<Inner_std140, 4u>) -> array<Inner, 4u> {
  var arr : array<Inner, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    arr[i] = conv_Inner(val[i]);
  }
  return arr;
}

fn conv_Outer(val : Outer_std140) -> Outer {
  return Outer(conv_arr4_Inner(val.a));
}

fn conv_arr4_Outer(val : array<Outer_std140, 4u>) -> array<Outer, 4u> {
  var arr : array<Outer, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    arr[i] = conv_Outer(val[i]);
  }
  return arr;
}

fn load_a_3_a_2_m() -> ${mat} {
  let s = &(a[3u].a[2u]);
  return ${mat}(${col_vectors_inline_load_matrix});
}

fn load_a_3_a_p0_m(p0 : u32) -> ${mat} {
  let s = &(a[3u].a[p0]);
  return ${mat}(${col_vectors_inline_load_matrix});
}

fn load_a_p0_a_2_m(p0 : u32) -> ${mat} {
  let s = &(a[p0].a[2u]);
  return ${mat}(${col_vectors_inline_load_matrix});
}

fn load_a_p0_a_p1_m(p0 : u32, p1 : u32) -> ${mat} {
  let s = &(a[p0].a[p1]);
  return ${mat}(${col_vectors_inline_load_matrix});
}

fn load_a_p0_a_p1_m_p2(p0 : u32, p1 : u32, p2 : u32) -> ${col_vector_type} {
  switch(p2) {
${col_table_load_column}
    default: {
      return ${col_vector_type}();
    }
  }
}

fn load_a_p0_a_p1_m_p2_p3(p0 : u32, p1 : u32, p2 : u32, p3 : u32) -> ${elem_type} {
  switch(p2) {
${col_table_load_element}
    default: {
      return ${elem_type}();
    }
  }
}

fn f() {
  let I = 1;
  let J = 2;
  let K = 0;
  let p_a = conv_arr4_Outer(a);
  let p_a_3 = conv_Outer(a[3u]);
  let p_a_I = conv_Outer(a[I]);
  let p_a_3_a = conv_arr4_Inner(a[3u].a);
  let p_a_I_a = conv_arr4_Inner(a[I].a);
  let p_a_3_a_2 = conv_Inner(a[3u].a[2u]);
  let p_a_3_a_I = conv_Inner(a[3u].a[I]);
  let p_a_I_a_2 = conv_Inner(a[I].a[2u]);
  let p_a_I_a_J = conv_Inner(a[I].a[J]);
  let p_a_3_a_2_m = load_a_3_a_2_m();
  let p_a_3_a_I_m = load_a_3_a_p0_m(u32(I));
  let p_a_I_a_2_m = load_a_p0_a_2_m(u32(I));
  let p_a_I_a_J_m = load_a_p0_a_p1_m(u32(I), u32(J));
  let p_a_3_a_2_m_1 = a[3u].a[2u].m_1;
  let p_a_I_a_J_m_K = load_a_p0_a_p1_m_p2(u32(I), u32(J), u32(K));
  let l_a : array<Outer, 4> = conv_arr4_Outer(a);
  let l_a_3 : Outer = conv_Outer(a[3u]);
  let l_a_I : Outer = conv_Outer(a[I]);
  let l_a_3_a : array<Inner, 4> = conv_arr4_Inner(a[3u].a);
  let l_a_I_a : array<Inner, 4> = conv_arr4_Inner(a[I].a);
  let l_a_3_a_2 : Inner = conv_Inner(a[3u].a[2u]);
  let l_a_3_a_I : Inner = conv_Inner(a[3u].a[I]);
  let l_a_I_a_2 : Inner = conv_Inner(a[I].a[2u]);
  let l_a_I_a_J : Inner = conv_Inner(a[I].a[J]);
  let l_a_3_a_2_m : ${mat} = load_a_3_a_2_m();
  let l_a_3_a_I_m : ${mat} = load_a_3_a_p0_m(u32(I));
  let l_a_I_a_2_m : ${mat} = load_a_p0_a_2_m(u32(I));
  let l_a_I_a_J_m : ${mat} = load_a_p0_a_p1_m(u32(I), u32(J));
  let l_a_3_a_2_m_1 : ${col_vector_type} = a[3u].a[2u].m_1;
  let l_a_I_a_J_m_K : ${col_vector_type} = load_a_p0_a_p1_m_p2(u32(I), u32(J), u32(K));
  let l_a_2_a_0_m_1_0 : ${elem_type} = a[3u].a[2u].m_1[0u];
  let l_a_I_a_J_m_K_I : ${elem_type} = load_a_p0_a_p1_m_p2_p3(u32(I), u32(J), u32(K), u32(I));
}
)";
        std::string col_tableLoadColumn = matrix.JoinTemplatedStringForEachMatrixColumn(  //
            R"(    case ${col_id_for_tmpl}u: {
      return a[p0].a[p1].m_${col_id_for_tmpl};
    })",
            "\n");
        std::string col_tableLoadElement = matrix.JoinTemplatedStringForEachMatrixColumn(  //
            R"(    case ${col_id_for_tmpl}u: {
      return a[p0].a[p1].m_${col_id_for_tmpl}[p3];
    })",
            "\n");
        uint32_t last_size =
            64 - static_cast<uint32_t>(matrix.ColumnVectorAlign() * (matrix.columns - 1));
        expect = matrix.ReplaceFieldsInString(
            expect,
            {{"${col_vectors}", matrix.ExpendedColumnVectorsWithLastSize(2, "m_", last_size)},
             {"${col_vectors_inline_conv_Inner}",
              matrix.ExpendedColumnVectorsInline("val.m_", ", ")},
             {"${col_vectors_inline_load_matrix}",
              matrix.ExpendedColumnVectorsInline("(*(s)).m_", ", ")},
             {"${col_table_load_column}", col_tableLoadColumn},
             {"${col_table_load_element}", col_tableLoadElement}});
    } else {
        expect = src;
    }

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_P(Std140Test_Matrix, ArrayStructMatUniform_CopyArray_UniformToStorage) {
    auto matrix = GetParam();

    std::string src = R"(
enable f16;

struct S {
  @size(64)
  m : ${mat},
}

@group(0) @binding(0) var<uniform> u : array<S, 4>;

@group(0) @binding(1) var<storage, read_write> s : array<S, 4>;

fn f() {
  s = u;
}
)";
    src = matrix.ReplaceFieldsInString(src);

    std::string expect;
    if (matrix.NotStd140Compatible()) {
        expect = R"(
enable f16;

struct S {
  @size(64)
  m : ${mat},
}

struct S_std140 {
${col_vectors}
}

@group(0) @binding(0) var<uniform> u : array<S_std140, 4u>;

@group(0) @binding(1) var<storage, read_write> s : array<S, 4>;

fn conv_S(val : S_std140) -> S {
  return S(${mat}(${col_vectors_inline}));
}

fn conv_arr4_S(val : array<S_std140, 4u>) -> array<S, 4u> {
  var arr : array<S, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    arr[i] = conv_S(val[i]);
  }
  return arr;
}

fn f() {
  s = conv_arr4_S(u);
}
)";
        uint32_t last_size =
            64 - static_cast<uint32_t>(matrix.ColumnVectorAlign() * (matrix.columns - 1));
        expect = matrix.ReplaceFieldsInString(
            expect,
            {{"${col_vectors}", matrix.ExpendedColumnVectorsWithLastSize(2, "m_", last_size)},
             {"${col_vectors_inline}", matrix.ExpendedColumnVectorsInline("val.m_", ", ")}});
    } else {
        expect = src;
    }

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_P(Std140Test_Matrix, ArrayStructMatUniform_CopyStruct_UniformToWorkgroup) {
    auto matrix = GetParam();

    std::string src = R"(
enable f16;

struct S {
  v : vec4<i32>,
  @size(64)
  m : ${mat},
}

@group(0) @binding(0) var<uniform> u : array<S, 4>;

var<workgroup> w : array<S, 4>;

fn f() {
  w[0] = u[1];
}
)";
    src = matrix.ReplaceFieldsInString(src);

    std::string expect;
    if (matrix.NotStd140Compatible()) {
        expect = R"(
enable f16;

struct S {
  v : vec4<i32>,
  @size(64)
  m : ${mat},
}

struct S_std140 {
  v : vec4<i32>,
${col_vectors}
}

@group(0) @binding(0) var<uniform> u : array<S_std140, 4u>;

var<workgroup> w : array<S, 4>;

fn conv_S(val : S_std140) -> S {
  return S(val.v, ${mat}(${col_vectors_inline}));
}

fn f() {
  w[0] = conv_S(u[1u]);
}
)";
        uint32_t last_size =
            64 - static_cast<uint32_t>(matrix.ColumnVectorAlign() * (matrix.columns - 1));
        expect = matrix.ReplaceFieldsInString(
            expect,
            {{"${col_vectors}", matrix.ExpendedColumnVectorsWithLastSize(2, "m_", last_size)},
             {"${col_vectors_inline}", matrix.ExpendedColumnVectorsInline("val.m_", ", ")}});
    } else {
        expect = src;
    }

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_P(Std140Test_Matrix, ArrayStructMatUniform_CopyMatrix_UniformToPrivate) {
    auto matrix = GetParam();

    std::string src = R"(
enable f16;

struct S {
  v : vec4<i32>,
  @size(64)
  m : ${mat},
}

@group(0) @binding(0) var<uniform> u : array<S, 3>;

var<private> p : array<S, 4>;

fn f() {
  p[2].m = u[1].m;
}
)";
    src = matrix.ReplaceFieldsInString(src);

    std::string expect;
    if (matrix.NotStd140Compatible()) {
        expect = R"(
enable f16;

struct S {
  v : vec4<i32>,
  @size(64)
  m : ${mat},
}

struct S_std140 {
  v : vec4<i32>,
${col_vectors}
}

@group(0) @binding(0) var<uniform> u : array<S_std140, 3u>;

var<private> p : array<S, 4>;

fn load_u_1_m() -> ${mat} {
  let s = &(u[1u]);
  return ${mat}(${col_vectors_inline});
}

fn f() {
  p[2].m = load_u_1_m();
}
)";
        uint32_t last_size =
            64 - static_cast<uint32_t>(matrix.ColumnVectorAlign() * (matrix.columns - 1));
        expect = matrix.ReplaceFieldsInString(
            expect,
            {{"${col_vectors}", matrix.ExpendedColumnVectorsWithLastSize(2, "m_", last_size)},
             {"${col_vectors_inline}", matrix.ExpendedColumnVectorsInline("(*(s)).m_", ", ")}});
    } else {
        expect = src;
    }

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_P(Std140Test_Matrix, ArrayStructMatUniform_CopyColumn_UniformToStorage) {
    auto matrix = GetParam();

    std::string src = R"(
enable f16;

struct S {
  @size(64)
  m : ${mat},
}

@group(0) @binding(0) var<uniform> u : array<S, 3>;

@group(0) @binding(1) var<storage, read_write> s : array<S, 4>;

fn f() {
  s[3].m[1] = u[2].m[0];
}
)";
    src = matrix.ReplaceFieldsInString(src);

    std::string expect;
    if (matrix.NotStd140Compatible()) {
        expect = R"(
enable f16;

struct S {
  @size(64)
  m : ${mat},
}

struct S_std140 {
${col_vectors}
}

@group(0) @binding(0) var<uniform> u : array<S_std140, 3u>;

@group(0) @binding(1) var<storage, read_write> s : array<S, 4>;

fn f() {
  s[3].m[1] = u[2u].m_0;
}
)";
        uint32_t last_size =
            64 - static_cast<uint32_t>(matrix.ColumnVectorAlign() * (matrix.columns - 1));
        expect = matrix.ReplaceFieldsInString(
            expect,
            {{"${col_vectors}", matrix.ExpendedColumnVectorsWithLastSize(2, "m_", last_size)}});
    } else {
        expect = src;
    }

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_P(Std140Test_Matrix, ArrayStructMatUniform_CopyColumnSwizzle_UniformToWorkgroup) {
    auto matrix = GetParam();

    std::string src = R"(
enable f16;

struct S {
  @size(64)
  m : ${mat},
}

@group(0) @binding(0) var<uniform> u : array<S, 4>;

var<workgroup> w : array<S, 4>;

fn f() {
  w[3].m[1] = u[2].m[0].${swizzle}.${swizzle};
}
)";
    src = matrix.ReplaceFieldsInString(src);

    std::string expect;
    if (matrix.NotStd140Compatible()) {
        expect = R"(
enable f16;

struct S {
  @size(64)
  m : ${mat},
}

struct S_std140 {
${col_vectors}
}

@group(0) @binding(0) var<uniform> u : array<S_std140, 4u>;

var<workgroup> w : array<S, 4>;

fn f() {
  w[3].m[1] = u[2u].m_0.${swizzle}.${swizzle};
}
)";
        uint32_t last_size =
            64 - static_cast<uint32_t>(matrix.ColumnVectorAlign() * (matrix.columns - 1));
        expect = matrix.ReplaceFieldsInString(
            expect,
            {{"${col_vectors}", matrix.ExpendedColumnVectorsWithLastSize(2, "m_", last_size)}});
    } else {
        expect = src;
    }

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_P(Std140Test_Matrix, ArrayStructMatUniform_CopyScalar_UniformToPrivate) {
    auto matrix = GetParam();

    std::string src = R"(
enable f16;

struct S {
  v : vec4<i32>,
  @size(64)
  m : ${mat},
}

@group(0) @binding(0) var<uniform> u : array<S, 3>;

var<private> p : array<S, 4>;

fn f() {
  p[3].m[1].x = u[2].m[0].y;
}
)";
    src = matrix.ReplaceFieldsInString(src);

    std::string expect;
    if (matrix.NotStd140Compatible()) {
        expect = R"(
enable f16;

struct S {
  v : vec4<i32>,
  @size(64)
  m : ${mat},
}

struct S_std140 {
  v : vec4<i32>,
${col_vectors}
}

@group(0) @binding(0) var<uniform> u : array<S_std140, 3u>;

var<private> p : array<S, 4>;

fn f() {
  p[3].m[1].x = u[2u].m_0[1u];
}
)";
        uint32_t last_size =
            64 - static_cast<uint32_t>(matrix.ColumnVectorAlign() * (matrix.columns - 1));
        expect = matrix.ReplaceFieldsInString(
            expect,
            {{"${col_vectors}", matrix.ExpendedColumnVectorsWithLastSize(2, "m_", last_size)},
             {"${col_vectors_inline}", matrix.ExpendedColumnVectorsInline("(*(s)).m_", ", ")}});
    } else {
        expect = src;
    }

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

INSTANTIATE_TEST_SUITE_P(,
                         Std140Test_Matrix,
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

using Std140Test_MatrixArray = TransformTestWithParam<MatrixCase>;

TEST_P(Std140Test_MatrixArray, ArrayMatUniform_LoadArray) {
    auto matrix = GetParam();

    if (!matrix.CanBeUsedAsUniformArrayElememts()) {
        // This permutation is invalid, skip the test.
        return;
    }

    std::string src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : array<${mat}, 3>;

fn f() {
  let l = a;
}
)";
    src = matrix.ReplaceFieldsInString(src);

    std::string expect;
    if (matrix.NotStd140Compatible()) {
        expect = R"(
enable f16;

struct mat${shape}_${elem_type} {
${col_vectors}
}

@group(0) @binding(0) var<uniform> a : array<mat${shape}_${elem_type}, 3u>;

fn conv_mat${shape}_${elem_type}(val : mat${shape}_${elem_type}) -> ${mat} {
  return ${mat}(${col_vectors_inline});
}

fn conv_arr3_mat${shape}_${elem_type}(val : array<mat${shape}_${elem_type}, 3u>) -> array<${mat}, 3u> {
  var arr : array<${mat}, 3u>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    arr[i] = conv_mat${shape}_${elem_type}(val[i]);
  }
  return arr;
}

fn f() {
  let l = conv_arr3_mat${shape}_${elem_type}(a);
}
)";
        expect = matrix.ReplaceFieldsInString(
            expect,
            {{"${col_vectors}", matrix.ExpendedColumnVectors(2, "col")},
             {"${col_vectors_inline}", matrix.ExpendedColumnVectorsInline("val.col", ", ")}});
    } else {
        expect = src;
    }

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_P(Std140Test_MatrixArray, ArrayMatUniform_LoadMatrix_ConstArrayIndex) {
    auto matrix = GetParam();

    if (!matrix.CanBeUsedAsUniformArrayElememts()) {
        // This permutation is invalid, skip the test.
        return;
    }

    std::string tmpl_src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : array<${mat}, 3>;

fn f() {
  let l = a[${array_index}];
}
)";
    tmpl_src = matrix.ReplaceFieldsInString(tmpl_src);

    std::string tmpl_expect;
    if (matrix.NotStd140Compatible()) {
        tmpl_expect = R"(
enable f16;

struct mat${shape}_${elem_type} {
${col_vectors}
}

@group(0) @binding(0) var<uniform> a : array<mat${shape}_${elem_type}, 3u>;

fn conv_mat${shape}_${elem_type}(val : mat${shape}_${elem_type}) -> ${mat} {
  return ${mat}(${col_vectors_inline});
}

fn f() {
  let l = conv_mat${shape}_${elem_type}(a[${array_index}u]);
}
)";
        tmpl_expect = matrix.ReplaceFieldsInString(
            tmpl_expect,
            {{"${col_vectors}", matrix.ExpendedColumnVectors(2, "col")},
             {"${col_vectors_inline}", matrix.ExpendedColumnVectorsInline("val.col", ", ")}});
    } else {
        tmpl_expect = tmpl_src;
    }

    for (uint32_t array_index = 0; array_index < 3; array_index++) {
        std::string src =
            utils::ReplaceAll(tmpl_src, "${array_index}", std::to_string(array_index));
        std::string expect =
            utils::ReplaceAll(tmpl_expect, "${array_index}", std::to_string(array_index));

        auto got = Run<Std140>(src);

        EXPECT_EQ(expect, str(got)) << "accessing array element " << array_index;
    }
}

TEST_P(Std140Test_MatrixArray, ArrayMatUniform_LoadMatrix_VariableArrayIndex) {
    auto matrix = GetParam();

    if (!matrix.CanBeUsedAsUniformArrayElememts()) {
        // This permutation is invalid, skip the test.
        return;
    }

    std::string src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : array<${mat}, 3>;

fn f() {
  let I = 1;
  let l = a[I];
}
)";
    src = matrix.ReplaceFieldsInString(src);

    std::string expect;
    if (matrix.NotStd140Compatible()) {
        expect = R"(
enable f16;

struct mat${shape}_${elem_type} {
${col_vectors}
}

@group(0) @binding(0) var<uniform> a : array<mat${shape}_${elem_type}, 3u>;

fn conv_mat${shape}_${elem_type}(val : mat${shape}_${elem_type}) -> ${mat} {
  return ${mat}(${col_vectors_inline});
}

fn f() {
  let I = 1;
  let l = conv_mat${shape}_${elem_type}(a[I]);
}
)";
        expect = matrix.ReplaceFieldsInString(
            expect,
            {{"${col_vectors}", matrix.ExpendedColumnVectors(2, "col")},
             {"${col_vectors_inline}", matrix.ExpendedColumnVectorsInline("val.col", ", ")}});
    } else {
        expect = src;
    }

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_P(Std140Test_MatrixArray, ArrayMatUniform_LoadColumn_ConstArrayIndex_ConstColumnIndex) {
    auto matrix = GetParam();

    if (!matrix.CanBeUsedAsUniformArrayElememts()) {
        // This permutation is invalid, skip the test.
        return;
    }

    std::string tmpl_src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : array<${mat}, 3>;

fn f() {
  let l = a[${array_index}][${cloumn_index}];
}
)";
    tmpl_src = matrix.ReplaceFieldsInString(tmpl_src);

    std::string tmpl_expect;
    if (matrix.NotStd140Compatible()) {
        tmpl_expect = R"(
enable f16;

struct mat${shape}_${elem_type} {
${col_vectors}
}

@group(0) @binding(0) var<uniform> a : array<mat${shape}_${elem_type}, 3u>;

fn f() {
  let l = a[${array_index}u].col${cloumn_index};
}
)";
        tmpl_expect = matrix.ReplaceFieldsInString(
            tmpl_expect, {{"${col_vectors}", matrix.ExpendedColumnVectors(2, "col")}});
    } else {
        tmpl_expect = tmpl_src;
    }

    for (uint32_t array_index = 0; array_index < 3; array_index++) {
        for (uint32_t col = 0; col < matrix.columns; col++) {
            std::string src =
                utils::ReplaceAll(tmpl_src, "${array_index}", std::to_string(array_index));
            src = utils::ReplaceAll(src, "${cloumn_index}", std::to_string(col));
            std::string expect =
                utils::ReplaceAll(tmpl_expect, "${array_index}", std::to_string(array_index));
            expect = utils::ReplaceAll(expect, "${cloumn_index}", std::to_string(col));

            auto got = Run<Std140>(src);

            EXPECT_EQ(expect, str(got))
                << "accessing array element " << array_index << " col " << col;
        }
    }
}

TEST_P(Std140Test_MatrixArray, ArrayMatUniform_LoadColumn_VariableArrayIndex_ConstColumnIndex) {
    auto matrix = GetParam();

    if (!matrix.CanBeUsedAsUniformArrayElememts()) {
        // This permutation is invalid, skip the test.
        return;
    }

    std::string tmpl_src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : array<${mat}, 3>;

fn f() {
  let I = 1;
  let l = a[I][${cloumn_index}];
}
)";
    tmpl_src = matrix.ReplaceFieldsInString(tmpl_src);

    std::string tmpl_expect;
    if (matrix.NotStd140Compatible()) {
        tmpl_expect = R"(
enable f16;

struct mat${shape}_${elem_type} {
${col_vectors}
}

@group(0) @binding(0) var<uniform> a : array<mat${shape}_${elem_type}, 3u>;

fn f() {
  let I = 1;
  let l = a[I].col${cloumn_index};
}
)";
        tmpl_expect = matrix.ReplaceFieldsInString(
            tmpl_expect, {{"${col_vectors}", matrix.ExpendedColumnVectors(2, "col")}});
    } else {
        tmpl_expect = tmpl_src;
    }

    for (uint32_t col = 0; col < matrix.columns; col++) {
        std::string src = utils::ReplaceAll(tmpl_src, "${cloumn_index}", std::to_string(col));
        std::string expect = utils::ReplaceAll(tmpl_expect, "${cloumn_index}", std::to_string(col));

        auto got = Run<Std140>(src);

        EXPECT_EQ(expect, str(got)) << "accessing col " << col;
    }
}

TEST_P(Std140Test_MatrixArray, ArrayMatUniform_LoadColumn_ConstArrayIndex_VariableColumnIndex) {
    auto matrix = GetParam();

    if (!matrix.CanBeUsedAsUniformArrayElememts()) {
        // This permutation is invalid, skip the test.
        return;
    }

    std::string tmpl_src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : array<${mat}, 3>;

fn f() {
  let I = 1;
  let l = a[${array_index}][I];
}
)";
    tmpl_src = matrix.ReplaceFieldsInString(tmpl_src);

    std::string tmpl_expect;
    if (matrix.NotStd140Compatible()) {
        tmpl_expect = R"(
enable f16;

struct mat${shape}_${elem_type} {
${col_vectors}
}

@group(0) @binding(0) var<uniform> a : array<mat${shape}_${elem_type}, 3u>;

fn load_a_${array_index}_p0(p0 : u32) -> ${col_vector_type} {
  switch(p0) {
${col_table}
    default: {
      return ${col_vector_type}();
    }
  }
}

fn f() {
  let I = 1;
  let l = load_a_${array_index}_p0(u32(I));
}
)";
        // col_table is the switch cases for all column index.
        // Example for a matrix having 2 columns:
        //   case 0u: {
        //     return a[${array_index}u].col0;
        //   }
        //   case 1u: {
        //     return a[${array_index}u].col1;
        //   }
        std::string col_table = matrix.JoinTemplatedStringForEachMatrixColumn(  //
            R"(    case ${col_id_for_tmpl}u: {
      return a[${array_index}u].col${col_id_for_tmpl};
    })",
            "\n");
        tmpl_expect = matrix.ReplaceFieldsInString(
            tmpl_expect, {{"${col_vectors}", matrix.ExpendedColumnVectors(2, "col")},
                          {"${col_table}", col_table}});
    } else {
        tmpl_expect = tmpl_src;
    }

    for (uint32_t array_index = 0; array_index < 3; array_index++) {
        std::string src =
            utils::ReplaceAll(tmpl_src, "${array_index}", std::to_string(array_index));
        std::string expect =
            utils::ReplaceAll(tmpl_expect, "${array_index}", std::to_string(array_index));

        auto got = Run<Std140>(src);

        EXPECT_EQ(expect, str(got)) << "accessing array element " << array_index;
    }
}

TEST_P(Std140Test_MatrixArray, ArrayMatUniform_LoadColumn_VariableArrayIndex_VariableColumnIndex) {
    auto matrix = GetParam();

    if (!matrix.CanBeUsedAsUniformArrayElememts()) {
        // This permutation is invalid, skip the test.
        return;
    }

    std::string src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : array<${mat}, 3>;

fn f() {
  let I = 1;
  let l = a[I][I];
}
)";
    src = matrix.ReplaceFieldsInString(src);

    std::string expect;
    if (matrix.NotStd140Compatible()) {
        expect = R"(
enable f16;

struct mat${shape}_${elem_type} {
${col_vectors}
}

@group(0) @binding(0) var<uniform> a : array<mat${shape}_${elem_type}, 3u>;

fn load_a_p0_p1(p0 : u32, p1 : u32) -> ${col_vector_type} {
  switch(p1) {
${col_table}
    default: {
      return ${col_vector_type}();
    }
  }
}

fn f() {
  let I = 1;
  let l = load_a_p0_p1(u32(I), u32(I));
}
)";
        // col_table is the switch cases for all column index.
        // Example for a matrix having 2 columns:
        //   case 0u: {
        //     return a[p0].col0;
        //   }
        //   case 1u: {
        //     return a[p0].col1;
        //   }
        std::string col_table = matrix.JoinTemplatedStringForEachMatrixColumn(  //
            R"(    case ${col_id_for_tmpl}u: {
      return a[p0].col${col_id_for_tmpl};
    })",
            "\n");
        expect = matrix.ReplaceFieldsInString(
            expect, {{"${col_vectors}", matrix.ExpendedColumnVectors(2, "col")},
                     {"${col_table}", col_table}});
    } else {
        expect = src;
    }

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_P(Std140Test_MatrixArray, StructArrayMatUniform_LoadStruct) {
    auto matrix = GetParam();

    if (!matrix.CanBeUsedAsUniformArrayElememts()) {
        // This permutation is invalid, skip the test.
        return;
    }

    std::string src = R"(
enable f16;

struct S {
  a : array<${mat}, 3>,
}

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  let l = s;
}
)";
    src = matrix.ReplaceFieldsInString(src);

    std::string expect;
    if (matrix.NotStd140Compatible()) {
        expect = R"(
enable f16;

struct mat${shape}_${elem_type} {
${col_vectors}
}

struct S {
  a : array<${mat}, 3>,
}

struct S_std140 {
  a : array<mat${shape}_${elem_type}, 3u>,
}

@group(0) @binding(0) var<uniform> s : S_std140;

fn conv_mat${shape}_${elem_type}(val : mat${shape}_${elem_type}) -> ${mat} {
  return ${mat}(${col_vectors_inline});
}

fn conv_arr3_mat${shape}_${elem_type}(val : array<mat${shape}_${elem_type}, 3u>) -> array<${mat}, 3u> {
  var arr : array<${mat}, 3u>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    arr[i] = conv_mat${shape}_${elem_type}(val[i]);
  }
  return arr;
}

fn conv_S(val : S_std140) -> S {
  return S(conv_arr3_mat${shape}_${elem_type}(val.a));
}

fn f() {
  let l = conv_S(s);
}
)";
        expect = matrix.ReplaceFieldsInString(
            expect,
            {{"${col_vectors}", matrix.ExpendedColumnVectors(2, "col")},
             {"${col_vectors_inline}", matrix.ExpendedColumnVectorsInline("val.col", ", ")}});
    } else {
        expect = src;
    }

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_P(Std140Test_MatrixArray, StructArrayMatUniform_LoadArray) {
    auto matrix = GetParam();

    if (!matrix.CanBeUsedAsUniformArrayElememts()) {
        // This permutation is invalid, skip the test.
        return;
    }

    std::string src = R"(
enable f16;

struct S {
  a : array<${mat}, 3>,
}

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  let l = s.a;
}
)";
    src = matrix.ReplaceFieldsInString(src);

    std::string expect;
    if (matrix.NotStd140Compatible()) {
        expect = R"(
enable f16;

struct mat${shape}_${elem_type} {
${col_vectors}
}

struct S {
  a : array<${mat}, 3>,
}

struct S_std140 {
  a : array<mat${shape}_${elem_type}, 3u>,
}

@group(0) @binding(0) var<uniform> s : S_std140;

fn conv_mat${shape}_${elem_type}(val : mat${shape}_${elem_type}) -> ${mat} {
  return ${mat}(${col_vectors_inline});
}

fn conv_arr3_mat${shape}_${elem_type}(val : array<mat${shape}_${elem_type}, 3u>) -> array<${mat}, 3u> {
  var arr : array<${mat}, 3u>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    arr[i] = conv_mat${shape}_${elem_type}(val[i]);
  }
  return arr;
}

fn f() {
  let l = conv_arr3_mat${shape}_${elem_type}(s.a);
}
)";
        expect = matrix.ReplaceFieldsInString(
            expect,
            {{"${col_vectors}", matrix.ExpendedColumnVectors(2, "col")},
             {"${col_vectors_inline}", matrix.ExpendedColumnVectorsInline("val.col", ", ")}});
    } else {
        expect = src;
    }

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_P(Std140Test_MatrixArray, StructArrayMatUniform_LoadMatrix_ConstArrayIndex) {
    auto matrix = GetParam();

    if (!matrix.CanBeUsedAsUniformArrayElememts()) {
        // This permutation is invalid, skip the test.
        return;
    }

    std::string tmpl_src = R"(
enable f16;

struct S {
  a : array<${mat}, 3>,
}

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  let l = s.a[${array_index}];
}
)";
    tmpl_src = matrix.ReplaceFieldsInString(tmpl_src);

    std::string tmpl_expect;
    if (matrix.NotStd140Compatible()) {
        tmpl_expect = R"(
enable f16;

struct mat${shape}_${elem_type} {
${col_vectors}
}

struct S {
  a : array<${mat}, 3>,
}

struct S_std140 {
  a : array<mat${shape}_${elem_type}, 3u>,
}

@group(0) @binding(0) var<uniform> s : S_std140;

fn conv_mat${shape}_${elem_type}(val : mat${shape}_${elem_type}) -> ${mat} {
  return ${mat}(${col_vectors_inline});
}

fn f() {
  let l = conv_mat${shape}_${elem_type}(s.a[${array_index}u]);
}
)";
        tmpl_expect = matrix.ReplaceFieldsInString(
            tmpl_expect,
            {{"${col_vectors}", matrix.ExpendedColumnVectors(2, "col")},
             {"${col_vectors_inline}", matrix.ExpendedColumnVectorsInline("val.col", ", ")}});
    } else {
        tmpl_expect = tmpl_src;
    }

    for (uint32_t array_index = 0; array_index < 3; array_index++) {
        std::string src =
            utils::ReplaceAll(tmpl_src, "${array_index}", std::to_string(array_index));
        std::string expect =
            utils::ReplaceAll(tmpl_expect, "${array_index}", std::to_string(array_index));

        auto got = Run<Std140>(src);

        EXPECT_EQ(expect, str(got)) << "accessing array element " << array_index;
    }
}

TEST_P(Std140Test_MatrixArray, StructArrayMatUniform_LoadMatrix_VariableArrayIndex) {
    auto matrix = GetParam();

    if (!matrix.CanBeUsedAsUniformArrayElememts()) {
        // This permutation is invalid, skip the test.
        return;
    }

    std::string src = R"(
enable f16;

struct S {
  a : array<${mat}, 3>,
}

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  let I = 1;
  let l = s.a[I];
}
)";
    src = matrix.ReplaceFieldsInString(src);

    std::string expect;
    if (matrix.NotStd140Compatible()) {
        expect = R"(
enable f16;

struct mat${shape}_${elem_type} {
${col_vectors}
}

struct S {
  a : array<${mat}, 3>,
}

struct S_std140 {
  a : array<mat${shape}_${elem_type}, 3u>,
}

@group(0) @binding(0) var<uniform> s : S_std140;

fn conv_mat${shape}_${elem_type}(val : mat${shape}_${elem_type}) -> ${mat} {
  return ${mat}(${col_vectors_inline});
}

fn f() {
  let I = 1;
  let l = conv_mat${shape}_${elem_type}(s.a[I]);
}
)";
        expect = matrix.ReplaceFieldsInString(
            expect,
            {{"${col_vectors}", matrix.ExpendedColumnVectors(2, "col")},
             {"${col_vectors_inline}", matrix.ExpendedColumnVectorsInline("val.col", ", ")}});
    } else {
        expect = src;
    }

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_P(Std140Test_MatrixArray, StructArrayMatUniform_LoadColumn_ConstArrayIndex_ConstColumnIndex) {
    auto matrix = GetParam();

    if (!matrix.CanBeUsedAsUniformArrayElememts()) {
        // This permutation is invalid, skip the test.
        return;
    }

    std::string tmpl_src = R"(
enable f16;

struct S {
  a : array<${mat}, 3>,
}

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  let l = s.a[${array_index}][${cloumn_index}];
}
)";
    tmpl_src = matrix.ReplaceFieldsInString(tmpl_src);

    std::string tmpl_expect;
    if (matrix.NotStd140Compatible()) {
        tmpl_expect = R"(
enable f16;

struct mat${shape}_${elem_type} {
${col_vectors}
}

struct S {
  a : array<${mat}, 3>,
}

struct S_std140 {
  a : array<mat${shape}_${elem_type}, 3u>,
}

@group(0) @binding(0) var<uniform> s : S_std140;

fn f() {
  let l = s.a[${array_index}u].col${cloumn_index};
}
)";
        tmpl_expect = matrix.ReplaceFieldsInString(
            tmpl_expect, {{"${col_vectors}", matrix.ExpendedColumnVectors(2, "col")}});
    } else {
        tmpl_expect = tmpl_src;
    }

    for (uint32_t array_index = 0; array_index < 3; array_index++) {
        for (uint32_t col = 0; col < matrix.columns; col++) {
            std::string src =
                utils::ReplaceAll(tmpl_src, "${array_index}", std::to_string(array_index));
            src = utils::ReplaceAll(src, "${cloumn_index}", std::to_string(col));
            std::string expect =
                utils::ReplaceAll(tmpl_expect, "${array_index}", std::to_string(array_index));
            expect = utils::ReplaceAll(expect, "${cloumn_index}", std::to_string(col));

            auto got = Run<Std140>(src);

            EXPECT_EQ(expect, str(got))
                << "accessing array element " << array_index << " col " << col;
        }
    }
}

TEST_P(Std140Test_MatrixArray,
       StructArrayMatUniform_LoadColumn_VariableArrayIndex_ConstColumnIndex) {
    auto matrix = GetParam();

    if (!matrix.CanBeUsedAsUniformArrayElememts()) {
        // This permutation is invalid, skip the test.
        return;
    }

    std::string tmpl_src = R"(
enable f16;

struct S {
  a : array<${mat}, 3>,
}

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  let I = 1;
  let l = s.a[I][${cloumn_index}];
}
)";
    tmpl_src = matrix.ReplaceFieldsInString(tmpl_src);

    std::string tmpl_expect;
    if (matrix.NotStd140Compatible()) {
        tmpl_expect = R"(
enable f16;

struct mat${shape}_${elem_type} {
${col_vectors}
}

struct S {
  a : array<${mat}, 3>,
}

struct S_std140 {
  a : array<mat${shape}_${elem_type}, 3u>,
}

@group(0) @binding(0) var<uniform> s : S_std140;

fn f() {
  let I = 1;
  let l = s.a[I].col${cloumn_index};
}
)";
        tmpl_expect = matrix.ReplaceFieldsInString(
            tmpl_expect, {{"${col_vectors}", matrix.ExpendedColumnVectors(2, "col")}});
    } else {
        tmpl_expect = tmpl_src;
    }

    for (uint32_t col = 0; col < matrix.columns; col++) {
        std::string src = utils::ReplaceAll(tmpl_src, "${cloumn_index}", std::to_string(col));
        std::string expect = utils::ReplaceAll(tmpl_expect, "${cloumn_index}", std::to_string(col));

        auto got = Run<Std140>(src);

        EXPECT_EQ(expect, str(got)) << "accessing col " << col;
    }
}

TEST_P(Std140Test_MatrixArray,
       StructArrayMatUniform_LoadColumn_ConstArrayIndex_VariableColumnIndex) {
    auto matrix = GetParam();

    if (!matrix.CanBeUsedAsUniformArrayElememts()) {
        // This permutation is invalid, skip the test.
        return;
    }

    std::string tmpl_src = R"(
enable f16;

struct S {
  a : array<${mat}, 3>,
}

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  let I = 1;
  let l = s.a[${array_index}][I];
}
)";
    tmpl_src = matrix.ReplaceFieldsInString(tmpl_src);

    std::string tmpl_expect;
    if (matrix.NotStd140Compatible()) {
        tmpl_expect = R"(
enable f16;

struct mat${shape}_${elem_type} {
${col_vectors}
}

struct S {
  a : array<${mat}, 3>,
}

struct S_std140 {
  a : array<mat${shape}_${elem_type}, 3u>,
}

@group(0) @binding(0) var<uniform> s : S_std140;

fn load_s_a_${array_index}_p0(p0 : u32) -> ${col_vector_type} {
  switch(p0) {
${col_table}
    default: {
      return ${col_vector_type}();
    }
  }
}

fn f() {
  let I = 1;
  let l = load_s_a_${array_index}_p0(u32(I));
}
)";
        // col_table is the switch cases for all column index.
        // Example for a matrix having 2 columns:
        //   case 0u: {
        //     return s.a[${array_index}u].col0;
        //   }
        //   case 1u: {
        //     return s.a[${array_index}u].col1;
        //   }
        std::string col_table = matrix.JoinTemplatedStringForEachMatrixColumn(  //
            R"(    case ${col_id_for_tmpl}u: {
      return s.a[${array_index}u].col${col_id_for_tmpl};
    })",
            "\n");
        tmpl_expect = matrix.ReplaceFieldsInString(
            tmpl_expect, {{"${col_vectors}", matrix.ExpendedColumnVectors(2, "col")},
                          {"${col_table}", col_table}});
    } else {
        tmpl_expect = tmpl_src;
    }

    for (uint32_t array_index = 0; array_index < 3; array_index++) {
        std::string src =
            utils::ReplaceAll(tmpl_src, "${array_index}", std::to_string(array_index));
        std::string expect =
            utils::ReplaceAll(tmpl_expect, "${array_index}", std::to_string(array_index));

        auto got = Run<Std140>(src);

        EXPECT_EQ(expect, str(got)) << "accessing array element " << array_index;
    }
}

TEST_P(Std140Test_MatrixArray,
       StructArrayMatUniform_LoadColumn_VariableArrayIndex_VariableColumnIndex) {
    auto matrix = GetParam();

    if (!matrix.CanBeUsedAsUniformArrayElememts()) {
        // This permutation is invalid, skip the test.
        return;
    }

    std::string src = R"(
enable f16;

struct S {
  a : array<${mat}, 3>,
}

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  let I = 1;
  let l = s.a[I][I];
}
)";
    src = matrix.ReplaceFieldsInString(src);

    std::string expect;
    if (matrix.NotStd140Compatible()) {
        expect = R"(
enable f16;

struct mat${shape}_${elem_type} {
${col_vectors}
}

struct S {
  a : array<${mat}, 3>,
}

struct S_std140 {
  a : array<mat${shape}_${elem_type}, 3u>,
}

@group(0) @binding(0) var<uniform> s : S_std140;

fn load_s_a_p0_p1(p0 : u32, p1 : u32) -> ${col_vector_type} {
  switch(p1) {
${col_table}
    default: {
      return ${col_vector_type}();
    }
  }
}

fn f() {
  let I = 1;
  let l = load_s_a_p0_p1(u32(I), u32(I));
}
)";
        // col_table is the switch cases for all column index.
        // Example for a matrix having 2 columns:
        //   case 0u: {
        //     return s.a[p0].col0;
        //   }
        //   case 1u: {
        //     return s.a[p0].col1;
        //   }
        std::string col_table = matrix.JoinTemplatedStringForEachMatrixColumn(  //
            R"(    case ${col_id_for_tmpl}u: {
      return s.a[p0].col${col_id_for_tmpl};
    })",
            "\n");
        expect = matrix.ReplaceFieldsInString(
            expect, {{"${col_vectors}", matrix.ExpendedColumnVectors(2, "col")},
                     {"${col_table}", col_table}});
    } else {
        expect = src;
    }

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_P(Std140Test_MatrixArray, ArrayArrayMatUniform_LoadArrays) {
    auto matrix = GetParam();

    if (!matrix.CanBeUsedAsUniformArrayElememts()) {
        // This permutation is invalid, skip the test.
        return;
    }

    std::string src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : array<array<${mat}, 3>, 4>;

fn f() {
  let l = a;
}
)";
    src = matrix.ReplaceFieldsInString(src);

    std::string expect;
    if (matrix.NotStd140Compatible()) {
        expect = R"(
enable f16;

struct mat${shape}_${elem_type} {
${col_vectors}
}

@group(0) @binding(0) var<uniform> a : array<array<mat${shape}_${elem_type}, 3u>, 4u>;

fn conv_mat${shape}_${elem_type}(val : mat${shape}_${elem_type}) -> ${mat} {
  return ${mat}(${col_vectors_inline});
}

fn conv_arr3_mat${shape}_${elem_type}(val : array<mat${shape}_${elem_type}, 3u>) -> array<${mat}, 3u> {
  var arr : array<${mat}, 3u>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    arr[i] = conv_mat${shape}_${elem_type}(val[i]);
  }
  return arr;
}

fn conv_arr4_arr3_mat${shape}_${elem_type}(val : array<array<mat${shape}_${elem_type}, 3u>, 4u>) -> array<array<${mat}, 3u>, 4u> {
  var arr : array<array<${mat}, 3u>, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    arr[i] = conv_arr3_mat${shape}_${elem_type}(val[i]);
  }
  return arr;
}

fn f() {
  let l = conv_arr4_arr3_mat${shape}_${elem_type}(a);
}
)";
        expect = matrix.ReplaceFieldsInString(
            expect,
            {{"${col_vectors}", matrix.ExpendedColumnVectors(2, "col")},
             {"${col_vectors_inline}", matrix.ExpendedColumnVectorsInline("val.col", ", ")}});
    } else {
        expect = src;
    }

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_P(Std140Test_MatrixArray, ArrayArrayMatUniform_LoadArray_ConstOuterArrayIndex) {
    auto matrix = GetParam();

    if (!matrix.CanBeUsedAsUniformArrayElememts()) {
        // This permutation is invalid, skip the test.
        return;
    }

    std::string tmpl_src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : array<array<${mat}, 3>, 4>;

fn f() {
  let l = a[${outer_array_index}];
}
)";
    tmpl_src = matrix.ReplaceFieldsInString(tmpl_src);

    std::string tmpl_expect;
    if (matrix.NotStd140Compatible()) {
        tmpl_expect = R"(
enable f16;

struct mat${shape}_${elem_type} {
${col_vectors}
}

@group(0) @binding(0) var<uniform> a : array<array<mat${shape}_${elem_type}, 3u>, 4u>;

fn conv_mat${shape}_${elem_type}(val : mat${shape}_${elem_type}) -> ${mat} {
  return ${mat}(${col_vectors_inline});
}

fn conv_arr3_mat${shape}_${elem_type}(val : array<mat${shape}_${elem_type}, 3u>) -> array<${mat}, 3u> {
  var arr : array<${mat}, 3u>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    arr[i] = conv_mat${shape}_${elem_type}(val[i]);
  }
  return arr;
}

fn f() {
  let l = conv_arr3_mat${shape}_${elem_type}(a[${outer_array_index}u]);
}
)";
        tmpl_expect = matrix.ReplaceFieldsInString(
            tmpl_expect,
            {{"${col_vectors}", matrix.ExpendedColumnVectors(2, "col")},
             {"${col_vectors_inline}", matrix.ExpendedColumnVectorsInline("val.col", ", ")}});
    } else {
        tmpl_expect = tmpl_src;
    }

    for (uint32_t outer = 0; outer < 4; outer++) {
        std::string src =
            utils::ReplaceAll(tmpl_src, "${outer_array_index}", std::to_string(outer));
        std::string expect =
            utils::ReplaceAll(tmpl_expect, "${outer_array_index}", std::to_string(outer));

        auto got = Run<Std140>(src);

        EXPECT_EQ(expect, str(got)) << "accessing array element " << outer;
    }
}

TEST_P(Std140Test_MatrixArray, ArrayArrayMatUniform_LoadArray_VariableOuterArrayIndex) {
    auto matrix = GetParam();

    if (!matrix.CanBeUsedAsUniformArrayElememts()) {
        // This permutation is invalid, skip the test.
        return;
    }

    std::string src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : array<array<${mat}, 3>, 4>;

fn f() {
  let I = 1;
  let l = a[I];
}
)";
    src = matrix.ReplaceFieldsInString(src);

    std::string expect;
    if (matrix.NotStd140Compatible()) {
        expect = R"(
enable f16;

struct mat${shape}_${elem_type} {
${col_vectors}
}

@group(0) @binding(0) var<uniform> a : array<array<mat${shape}_${elem_type}, 3u>, 4u>;

fn conv_mat${shape}_${elem_type}(val : mat${shape}_${elem_type}) -> ${mat} {
  return ${mat}(${col_vectors_inline});
}

fn conv_arr3_mat${shape}_${elem_type}(val : array<mat${shape}_${elem_type}, 3u>) -> array<${mat}, 3u> {
  var arr : array<${mat}, 3u>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    arr[i] = conv_mat${shape}_${elem_type}(val[i]);
  }
  return arr;
}

fn f() {
  let I = 1;
  let l = conv_arr3_mat${shape}_${elem_type}(a[I]);
}
)";
        expect = matrix.ReplaceFieldsInString(
            expect,
            {{"${col_vectors}", matrix.ExpendedColumnVectors(2, "col")},
             {"${col_vectors_inline}", matrix.ExpendedColumnVectorsInline("val.col", ", ")}});
    } else {
        expect = src;
    }

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_P(Std140Test_MatrixArray,
       ArrayArrayMatUniform_LoadMatrix_ConstOuterArrayIndex_ConstInnerArrayIndex) {
    auto matrix = GetParam();

    if (!matrix.CanBeUsedAsUniformArrayElememts()) {
        // This permutation is invalid, skip the test.
        return;
    }

    std::string tmpl_src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : array<array<${mat}, 3>, 4>;

fn f() {
  let l = a[${outer_array_index}][${inner_array_index}];
}
)";
    tmpl_src = matrix.ReplaceFieldsInString(tmpl_src);

    std::string tmpl_expect;
    if (matrix.NotStd140Compatible()) {
        tmpl_expect = R"(
enable f16;

struct mat${shape}_${elem_type} {
${col_vectors}
}

@group(0) @binding(0) var<uniform> a : array<array<mat${shape}_${elem_type}, 3u>, 4u>;

fn conv_mat${shape}_${elem_type}(val : mat${shape}_${elem_type}) -> ${mat} {
  return ${mat}(${col_vectors_inline});
}

fn f() {
  let l = conv_mat${shape}_${elem_type}(a[${outer_array_index}u][${inner_array_index}u]);
}
)";
        tmpl_expect = matrix.ReplaceFieldsInString(
            tmpl_expect,
            {{"${col_vectors}", matrix.ExpendedColumnVectors(2, "col")},
             {"${col_vectors_inline}", matrix.ExpendedColumnVectorsInline("val.col", ", ")}});
    } else {
        tmpl_expect = tmpl_src;
    }

    for (uint32_t outer = 0; outer < 4; outer++) {
        for (uint32_t inner = 0; inner < 3; inner++) {
            std::string src =
                utils::ReplaceAll(tmpl_src, "${outer_array_index}", std::to_string(outer));
            src = utils::ReplaceAll(src, "${inner_array_index}", std::to_string(inner));
            std::string expect =
                utils::ReplaceAll(tmpl_expect, "${outer_array_index}", std::to_string(outer));
            expect = utils::ReplaceAll(expect, "${inner_array_index}", std::to_string(inner));

            auto got = Run<Std140>(src);

            EXPECT_EQ(expect, str(got))
                << "accessing array element [" << outer << "][" << inner << "]";
        }
    }
}

TEST_P(Std140Test_MatrixArray,
       ArrayArrayMatUniform_LoadMatrix_ConstOuterArrayIndex_VariableInnerArrayIndex) {
    auto matrix = GetParam();

    if (!matrix.CanBeUsedAsUniformArrayElememts()) {
        // This permutation is invalid, skip the test.
        return;
    }

    std::string tmpl_src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : array<array<${mat}, 3>, 4>;

fn f() {
  let I = 1;
  let l = a[${outer_array_index}][I];
}
)";
    tmpl_src = matrix.ReplaceFieldsInString(tmpl_src);

    std::string tmpl_expect;
    if (matrix.NotStd140Compatible()) {
        tmpl_expect = R"(
enable f16;

struct mat${shape}_${elem_type} {
${col_vectors}
}

@group(0) @binding(0) var<uniform> a : array<array<mat${shape}_${elem_type}, 3u>, 4u>;

fn conv_mat${shape}_${elem_type}(val : mat${shape}_${elem_type}) -> ${mat} {
  return ${mat}(${col_vectors_inline});
}

fn f() {
  let I = 1;
  let l = conv_mat${shape}_${elem_type}(a[${outer_array_index}u][I]);
}
)";
        tmpl_expect = matrix.ReplaceFieldsInString(
            tmpl_expect,
            {{"${col_vectors}", matrix.ExpendedColumnVectors(2, "col")},
             {"${col_vectors_inline}", matrix.ExpendedColumnVectorsInline("val.col", ", ")}});
    } else {
        tmpl_expect = tmpl_src;
    }

    for (uint32_t outer = 0; outer < 4; outer++) {
        std::string src =
            utils::ReplaceAll(tmpl_src, "${outer_array_index}", std::to_string(outer));
        std::string expect =
            utils::ReplaceAll(tmpl_expect, "${outer_array_index}", std::to_string(outer));

        auto got = Run<Std140>(src);

        EXPECT_EQ(expect, str(got)) << "accessing array element [" << outer << "][I]";
    }
}

TEST_P(Std140Test_MatrixArray,
       ArrayArrayMatUniform_LoadMatrix_VariableOuterArrayIndex_ConstInnerArrayIndex) {
    auto matrix = GetParam();

    if (!matrix.CanBeUsedAsUniformArrayElememts()) {
        // This permutation is invalid, skip the test.
        return;
    }

    std::string tmpl_src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : array<array<${mat}, 3>, 4>;

fn f() {
  let I = 1;
  let l = a[I][${inner_array_index}];
}
)";
    tmpl_src = matrix.ReplaceFieldsInString(tmpl_src);

    std::string tmpl_expect;
    if (matrix.NotStd140Compatible()) {
        tmpl_expect = R"(
enable f16;

struct mat${shape}_${elem_type} {
${col_vectors}
}

@group(0) @binding(0) var<uniform> a : array<array<mat${shape}_${elem_type}, 3u>, 4u>;

fn conv_mat${shape}_${elem_type}(val : mat${shape}_${elem_type}) -> ${mat} {
  return ${mat}(${col_vectors_inline});
}

fn f() {
  let I = 1;
  let l = conv_mat${shape}_${elem_type}(a[I][${inner_array_index}u]);
}
)";
        tmpl_expect = matrix.ReplaceFieldsInString(
            tmpl_expect,
            {{"${col_vectors}", matrix.ExpendedColumnVectors(2, "col")},
             {"${col_vectors_inline}", matrix.ExpendedColumnVectorsInline("val.col", ", ")}});
    } else {
        tmpl_expect = tmpl_src;
    }

    for (uint32_t inner = 0; inner < 3; inner++) {
        std::string src =
            utils::ReplaceAll(tmpl_src, "${inner_array_index}", std::to_string(inner));
        std::string expect =
            utils::ReplaceAll(tmpl_expect, "${inner_array_index}", std::to_string(inner));

        auto got = Run<Std140>(src);

        EXPECT_EQ(expect, str(got)) << "accessing array element [I][" << inner << "]";
    }
}

TEST_P(Std140Test_MatrixArray,
       ArrayArrayMatUniform_LoadMatrix_VariableOuterArrayIndex_VariableInnerArrayIndex) {
    auto matrix = GetParam();

    if (!matrix.CanBeUsedAsUniformArrayElememts()) {
        // This permutation is invalid, skip the test.
        return;
    }

    std::string src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : array<array<${mat}, 3>, 4>;

fn f() {
  let I = 1;
  let l = a[I][I];
}
)";
    src = matrix.ReplaceFieldsInString(src);

    std::string expect;
    if (matrix.NotStd140Compatible()) {
        expect = R"(
enable f16;

struct mat${shape}_${elem_type} {
${col_vectors}
}

@group(0) @binding(0) var<uniform> a : array<array<mat${shape}_${elem_type}, 3u>, 4u>;

fn conv_mat${shape}_${elem_type}(val : mat${shape}_${elem_type}) -> ${mat} {
  return ${mat}(${col_vectors_inline});
}

fn f() {
  let I = 1;
  let l = conv_mat${shape}_${elem_type}(a[I][I]);
}
)";
        expect = matrix.ReplaceFieldsInString(
            expect,
            {{"${col_vectors}", matrix.ExpendedColumnVectors(2, "col")},
             {"${col_vectors_inline}", matrix.ExpendedColumnVectorsInline("val.col", ", ")}});
    } else {
        expect = src;
    }

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_P(Std140Test_MatrixArray,
       ArrayArrayMatUniform_LoadColumn_ConstOuterArrayIndex_ConstInnerArrayIndex_ConstColumnIndex) {
    auto matrix = GetParam();

    if (!matrix.CanBeUsedAsUniformArrayElememts()) {
        // This permutation is invalid, skip the test.
        return;
    }

    std::string tmpl_src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : array<array<${mat}, 3>, 4>;

fn f() {
  let l = a[${outer_array_index}][${inner_array_index}][${column_index}];
}
)";
    tmpl_src = matrix.ReplaceFieldsInString(tmpl_src);

    std::string tmpl_expect;
    if (matrix.NotStd140Compatible()) {
        tmpl_expect = R"(
enable f16;

struct mat${shape}_${elem_type} {
${col_vectors}
}

@group(0) @binding(0) var<uniform> a : array<array<mat${shape}_${elem_type}, 3u>, 4u>;

fn f() {
  let l = a[${outer_array_index}u][${inner_array_index}u].col${column_index};
}
)";
        tmpl_expect = matrix.ReplaceFieldsInString(
            tmpl_expect,
            {{"${col_vectors}", matrix.ExpendedColumnVectors(2, "col")},
             {"${col_vectors_inline}", matrix.ExpendedColumnVectorsInline("val.col", ", ")}});
    } else {
        tmpl_expect = tmpl_src;
    }

    for (uint32_t outer = 0; outer < 4; outer++) {
        for (uint32_t inner = 0; inner < 3; inner++) {
            for (uint32_t col = 0; col < matrix.columns; col++) {
                std::string src =
                    utils::ReplaceAll(tmpl_src, "${outer_array_index}", std::to_string(outer));
                src = utils::ReplaceAll(src, "${inner_array_index}", std::to_string(inner));
                src = utils::ReplaceAll(src, "${column_index}", std::to_string(col));
                std::string expect =
                    utils::ReplaceAll(tmpl_expect, "${outer_array_index}", std::to_string(outer));
                expect = utils::ReplaceAll(expect, "${inner_array_index}", std::to_string(inner));
                expect = utils::ReplaceAll(expect, "${column_index}", std::to_string(col));

                auto got = Run<Std140>(src);

                EXPECT_EQ(expect, str(got))
                    << "accessing array element [" << outer << "][" << inner << "] col " << col;
            }
        }
    }
}

TEST_P(
    Std140Test_MatrixArray,
    ArrayArrayMatUniform_LoadColumn_ConstOuterArrayIndex_ConstInnerArrayIndex_VariableColumnIndex) {
    auto matrix = GetParam();

    if (!matrix.CanBeUsedAsUniformArrayElememts()) {
        // This permutation is invalid, skip the test.
        return;
    }

    std::string tmpl_src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : array<array<${mat}, 3>, 4>;

fn f() {
  let I = 1;
  let l = a[${outer_array_index}][${inner_array_index}][I];
}
)";
    tmpl_src = matrix.ReplaceFieldsInString(tmpl_src);

    std::string tmpl_expect;
    if (matrix.NotStd140Compatible()) {
        tmpl_expect = R"(
enable f16;

struct mat${shape}_${elem_type} {
${col_vectors}
}

@group(0) @binding(0) var<uniform> a : array<array<mat${shape}_${elem_type}, 3u>, 4u>;

fn load_a_${outer_array_index}_${inner_array_index}_p0(p0 : u32) -> ${col_vector_type} {
  switch(p0) {
${col_table}
    default: {
      return ${col_vector_type}();
    }
  }
}

fn f() {
  let I = 1;
  let l = load_a_${outer_array_index}_${inner_array_index}_p0(u32(I));
}
)";
        // col_table is the switch cases for all column index.
        // Example for a matrix having 2 columns:
        //   case 0u: {
        //     return a[${outer_array_index}u][${inner_array_index}u].col0;
        //   }
        //   case 1u: {
        //     return a[${outer_array_index}u][${inner_array_index}u].col1;
        //   }
        std::string col_table = matrix.JoinTemplatedStringForEachMatrixColumn(  //
            R"(    case ${col_id_for_tmpl}u: {
      return a[${outer_array_index}u][${inner_array_index}u].col${col_id_for_tmpl};
    })",
            "\n");
        tmpl_expect = matrix.ReplaceFieldsInString(
            tmpl_expect, {{"${col_vectors}", matrix.ExpendedColumnVectors(2, "col")},
                          {"${col_table}", col_table}});
    } else {
        tmpl_expect = tmpl_src;
    }

    for (uint32_t outer = 0; outer < 4; outer++) {
        for (uint32_t inner = 0; inner < 3; inner++) {
            std::string src =
                utils::ReplaceAll(tmpl_src, "${outer_array_index}", std::to_string(outer));
            src = utils::ReplaceAll(src, "${inner_array_index}", std::to_string(inner));
            std::string expect =
                utils::ReplaceAll(tmpl_expect, "${outer_array_index}", std::to_string(outer));
            expect = utils::ReplaceAll(expect, "${inner_array_index}", std::to_string(inner));

            auto got = Run<Std140>(src);

            EXPECT_EQ(expect, str(got))
                << "accessing array element [" << outer << "][" << inner << "]";
        }
    }
}

TEST_P(
    Std140Test_MatrixArray,
    ArrayArrayMatUniform_LoadColumn_ConstOuterArrayIndex_VariableInnerArrayIndex_ConstColumnIndex) {
    auto matrix = GetParam();

    if (!matrix.CanBeUsedAsUniformArrayElememts()) {
        // This permutation is invalid, skip the test.
        return;
    }

    std::string tmpl_src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : array<array<${mat}, 3>, 4>;

fn f() {
  let I = 1;
  let l = a[${outer_array_index}][I][${column_index}];
}
)";
    tmpl_src = matrix.ReplaceFieldsInString(tmpl_src);

    std::string tmpl_expect;
    if (matrix.NotStd140Compatible()) {
        tmpl_expect = R"(
enable f16;

struct mat${shape}_${elem_type} {
${col_vectors}
}

@group(0) @binding(0) var<uniform> a : array<array<mat${shape}_${elem_type}, 3u>, 4u>;

fn f() {
  let I = 1;
  let l = a[${outer_array_index}u][I].col${column_index};
}
)";
        tmpl_expect = matrix.ReplaceFieldsInString(
            tmpl_expect, {{"${col_vectors}", matrix.ExpendedColumnVectors(2, "col")}});
    } else {
        tmpl_expect = tmpl_src;
    }

    for (uint32_t outer = 0; outer < 4; outer++) {
        for (uint32_t col = 0; col < matrix.columns; col++) {
            std::string src =
                utils::ReplaceAll(tmpl_src, "${outer_array_index}", std::to_string(outer));
            src = utils::ReplaceAll(src, "${column_index}", std::to_string(col));
            std::string expect =
                utils::ReplaceAll(tmpl_expect, "${outer_array_index}", std::to_string(outer));
            expect = utils::ReplaceAll(expect, "${column_index}", std::to_string(col));

            auto got = Run<Std140>(src);

            EXPECT_EQ(expect, str(got))
                << "accessing array element [" << outer << "][I] col " << col;
        }
    }
}

TEST_P(
    Std140Test_MatrixArray,
    ArrayArrayMatUniform_LoadColumn_ConstOuterArrayIndex_VariableInnerArrayIndex_VariableColumnIndex) {
    auto matrix = GetParam();

    if (!matrix.CanBeUsedAsUniformArrayElememts()) {
        // This permutation is invalid, skip the test.
        return;
    }

    std::string tmpl_src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : array<array<${mat}, 3>, 4>;

fn f() {
  let I = 1;
  let J = 2;
  let l = a[${outer_array_index}][I][J];
}
)";
    tmpl_src = matrix.ReplaceFieldsInString(tmpl_src);

    std::string tmpl_expect;
    if (matrix.NotStd140Compatible()) {
        tmpl_expect = R"(
enable f16;

struct mat${shape}_${elem_type} {
${col_vectors}
}

@group(0) @binding(0) var<uniform> a : array<array<mat${shape}_${elem_type}, 3u>, 4u>;

fn load_a_${outer_array_index}_p0_p1(p0 : u32, p1 : u32) -> ${col_vector_type} {
  switch(p1) {
${col_table}
    default: {
      return ${col_vector_type}();
    }
  }
}

fn f() {
  let I = 1;
  let J = 2;
  let l = load_a_${outer_array_index}_p0_p1(u32(I), u32(J));
}
)";
        // col_table is the switch cases for all column index.
        // Example for a matrix having 2 columns:
        //   case 0u: {
        //     return a[${outer_array_index}u][p0].col0;
        //   }
        //   case 1u: {
        //     return a[${outer_array_index}u][p0].col1;
        //   }
        std::string col_table = matrix.JoinTemplatedStringForEachMatrixColumn(  //
            R"(    case ${col_id_for_tmpl}u: {
      return a[${outer_array_index}u][p0].col${col_id_for_tmpl};
    })",
            "\n");
        tmpl_expect = matrix.ReplaceFieldsInString(
            tmpl_expect, {{"${col_vectors}", matrix.ExpendedColumnVectors(2, "col")},
                          {"${col_table}", col_table}});
    } else {
        tmpl_expect = tmpl_src;
    }

    for (uint32_t outer = 0; outer < 4; outer++) {
        std::string src =
            utils::ReplaceAll(tmpl_src, "${outer_array_index}", std::to_string(outer));
        std::string expect =
            utils::ReplaceAll(tmpl_expect, "${outer_array_index}", std::to_string(outer));

        auto got = Run<Std140>(src);

        EXPECT_EQ(expect, str(got)) << "accessing array element [" << outer << "][I]";
    }
}

TEST_P(
    Std140Test_MatrixArray,
    ArrayArrayMatUniform_LoadColumn_VariableOuterArrayIndex_ConstInnerArrayIndex_ConstColumnIndex) {
    auto matrix = GetParam();

    if (!matrix.CanBeUsedAsUniformArrayElememts()) {
        // This permutation is invalid, skip the test.
        return;
    }

    std::string tmpl_src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : array<array<${mat}, 3>, 4>;

fn f() {
  let I = 1;
  let l = a[I][${inner_array_index}][${column_index}];
}
)";
    tmpl_src = matrix.ReplaceFieldsInString(tmpl_src);

    std::string tmpl_expect;
    if (matrix.NotStd140Compatible()) {
        tmpl_expect = R"(
enable f16;

struct mat${shape}_${elem_type} {
${col_vectors}
}

@group(0) @binding(0) var<uniform> a : array<array<mat${shape}_${elem_type}, 3u>, 4u>;

fn f() {
  let I = 1;
  let l = a[I][${inner_array_index}u].col${column_index};
}
)";
        tmpl_expect = matrix.ReplaceFieldsInString(
            tmpl_expect, {{"${col_vectors}", matrix.ExpendedColumnVectors(2, "col")}});
    } else {
        tmpl_expect = tmpl_src;
    }

    for (uint32_t inner = 0; inner < 3; inner++) {
        for (uint32_t col = 0; col < matrix.columns; col++) {
            std::string src =
                utils::ReplaceAll(tmpl_src, "${inner_array_index}", std::to_string(inner));
            src = utils::ReplaceAll(src, "${column_index}", std::to_string(col));
            std::string expect =
                utils::ReplaceAll(tmpl_expect, "${inner_array_index}", std::to_string(inner));
            expect = utils::ReplaceAll(expect, "${column_index}", std::to_string(col));

            auto got = Run<Std140>(src);

            EXPECT_EQ(expect, str(got))
                << "accessing array element [I][" << inner << "] col " << col;
        }
    }
}

TEST_P(
    Std140Test_MatrixArray,
    ArrayArrayMatUniform_LoadColumn_VariableOuterArrayIndex_ConstInnerArrayIndex_VariableColumnIndex) {
    auto matrix = GetParam();

    if (!matrix.CanBeUsedAsUniformArrayElememts()) {
        // This permutation is invalid, skip the test.
        return;
    }

    std::string tmpl_src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : array<array<${mat}, 3>, 4>;

fn f() {
  let I = 1;
  let J = 2;
  let l = a[I][${inner_array_index}][J];
}
)";
    tmpl_src = matrix.ReplaceFieldsInString(tmpl_src);

    std::string tmpl_expect;
    if (matrix.NotStd140Compatible()) {
        tmpl_expect = R"(
enable f16;

struct mat${shape}_${elem_type} {
${col_vectors}
}

@group(0) @binding(0) var<uniform> a : array<array<mat${shape}_${elem_type}, 3u>, 4u>;

fn load_a_p0_${inner_array_index}_p1(p0 : u32, p1 : u32) -> ${col_vector_type} {
  switch(p1) {
${col_table}
    default: {
      return ${col_vector_type}();
    }
  }
}

fn f() {
  let I = 1;
  let J = 2;
  let l = load_a_p0_${inner_array_index}_p1(u32(I), u32(J));
}
)";
        // col_table is the switch cases for all column index.
        // Example for a matrix having 2 columns:
        //   case 0u: {
        //     return a[p0][${inner_array_index}u].col0;
        //   }
        //   case 1u: {
        //     return a[p0][${inner_array_index}u].col1;
        //   }
        std::string col_table = matrix.JoinTemplatedStringForEachMatrixColumn(  //
            R"(    case ${col_id_for_tmpl}u: {
      return a[p0][${inner_array_index}u].col${col_id_for_tmpl};
    })",
            "\n");
        tmpl_expect = matrix.ReplaceFieldsInString(
            tmpl_expect, {{"${col_vectors}", matrix.ExpendedColumnVectors(2, "col")},
                          {"${col_table}", col_table}});
    } else {
        tmpl_expect = tmpl_src;
    }

    for (uint32_t inner = 0; inner < 3; inner++) {
        std::string src =
            utils::ReplaceAll(tmpl_src, "${inner_array_index}", std::to_string(inner));
        std::string expect =
            utils::ReplaceAll(tmpl_expect, "${inner_array_index}", std::to_string(inner));

        auto got = Run<Std140>(src);

        EXPECT_EQ(expect, str(got)) << "accessing array element [I][" << inner << "]";
    }
}

TEST_P(
    Std140Test_MatrixArray,
    ArrayArrayMatUniform_LoadColumn_VariableOuterArrayIndex_VariableInnerArrayIndex_ConstColumnIndex) {
    auto matrix = GetParam();

    if (!matrix.CanBeUsedAsUniformArrayElememts()) {
        // This permutation is invalid, skip the test.
        return;
    }

    std::string tmpl_src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : array<array<${mat}, 3>, 4>;

fn f() {
  let I = 1;
  let J = 2;
  let l = a[I][J][${column_index}];
}
)";
    tmpl_src = matrix.ReplaceFieldsInString(tmpl_src);

    std::string tmpl_expect;
    if (matrix.NotStd140Compatible()) {
        tmpl_expect = R"(
enable f16;

struct mat${shape}_${elem_type} {
${col_vectors}
}

@group(0) @binding(0) var<uniform> a : array<array<mat${shape}_${elem_type}, 3u>, 4u>;

fn f() {
  let I = 1;
  let J = 2;
  let l = a[I][J].col${column_index};
}
)";
        tmpl_expect = matrix.ReplaceFieldsInString(
            tmpl_expect, {{"${col_vectors}", matrix.ExpendedColumnVectors(2, "col")}});
    } else {
        tmpl_expect = tmpl_src;
    }

    for (uint32_t col = 0; col < matrix.columns; col++) {
        std::string src = utils::ReplaceAll(tmpl_src, "${column_index}", std::to_string(col));
        std::string expect = utils::ReplaceAll(tmpl_expect, "${column_index}", std::to_string(col));

        auto got = Run<Std140>(src);

        EXPECT_EQ(expect, str(got)) << "accessing array element [I][J] col " << col;
    }
}

TEST_P(
    Std140Test_MatrixArray,
    ArrayArrayMatUniform_LoadColumn_VariableOuterArrayIndex_VariableInnerArrayIndex_VariableColumnIndex) {
    auto matrix = GetParam();

    if (!matrix.CanBeUsedAsUniformArrayElememts()) {
        // This permutation is invalid, skip the test.
        return;
    }

    std::string src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : array<array<${mat}, 3>, 4>;

fn f() {
  let I = 0;
  let J = 1;
  let K = 2;
  let l = a[I][J][K];
}
)";
    src = matrix.ReplaceFieldsInString(src);

    std::string expect;
    if (matrix.NotStd140Compatible()) {
        expect = R"(
enable f16;

struct mat${shape}_${elem_type} {
${col_vectors}
}

@group(0) @binding(0) var<uniform> a : array<array<mat${shape}_${elem_type}, 3u>, 4u>;

fn load_a_p0_p1_p2(p0 : u32, p1 : u32, p2 : u32) -> ${col_vector_type} {
  switch(p2) {
${col_table}
    default: {
      return ${col_vector_type}();
    }
  }
}

fn f() {
  let I = 0;
  let J = 1;
  let K = 2;
  let l = load_a_p0_p1_p2(u32(I), u32(J), u32(K));
}
)";
        // col_table is the switch cases for all column index.
        // Example for a matrix having 2 columns:
        //   case 0u: {
        //     return a[p0][p1].col0;
        //   }
        //   case 1u: {
        //     return a[p0][p1].col1;
        //   }
        std::string col_table = matrix.JoinTemplatedStringForEachMatrixColumn(  //
            R"(    case ${col_id_for_tmpl}u: {
      return a[p0][p1].col${col_id_for_tmpl};
    })",
            "\n");
        expect = matrix.ReplaceFieldsInString(
            expect, {{"${col_vectors}", matrix.ExpendedColumnVectors(2, "col")},
                     {"${col_table}", col_table}});
    } else {
        expect = src;
    }

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

INSTANTIATE_TEST_SUITE_P(,
                         Std140Test_MatrixArray,
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

}  // namespace
}  // namespace tint::transform
