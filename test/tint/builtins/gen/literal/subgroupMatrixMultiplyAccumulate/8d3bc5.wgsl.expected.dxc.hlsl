#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_result_i8_8x8 = Matrix<ComponentType::I8, 8, 8, MatrixUse::Accumulator, MatrixScope::Wave>;
using Matrix_left_u8_8x8 = Matrix<ComponentType::U8, 8, 8, MatrixUse::A, MatrixScope::Wave>;
using Matrix_right_u8_8x8 = Matrix<ComponentType::U8, 8, 8, MatrixUse::B, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
Matrix_result_i8_8x8 tint_MatrixMultiplyAccumulate(Matrix_left_u8_8x8 a, Matrix_right_u8_8x8 b, Matrix_result_i8_8x8 c) {
  Matrix_result_i8_8x8 acc = c;
  acc.MultiplyAccumulate(a, b);
  return acc;
}

Matrix_result_i8_8x8 subgroupMatrixMultiplyAccumulate_8d3bc5() {
  Matrix_result_i8_8x8 res = tint_MatrixMultiplyAccumulate(Matrix_left_u8_8x8::Splat(0u), Matrix_right_u8_8x8::Splat(0u), Matrix_result_i8_8x8::Splat(int(0)));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixMultiplyAccumulate_8d3bc5().Store(prevent_dce, 0u, 64u, MatrixLayout::RowMajor);
}

