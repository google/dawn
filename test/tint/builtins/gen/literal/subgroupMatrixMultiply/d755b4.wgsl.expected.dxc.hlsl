#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_result_i8_8x8 = Matrix<ComponentType::I8, 8, 8, MatrixUse::Accumulator, MatrixScope::Wave>;
using Matrix_left_u8_8x8 = Matrix<ComponentType::U8, 8, 8, MatrixUse::A, MatrixScope::Wave>;
using Matrix_right_u8_8x8 = Matrix<ComponentType::U8, 8, 8, MatrixUse::B, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
Matrix_result_i8_8x8 subgroupMatrixMultiply_d755b4() {
  Matrix_result_i8_8x8 res = Multiply<ComponentType::I8>(Matrix_left_u8_8x8::Splat(0u), Matrix_right_u8_8x8::Splat(0u));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixMultiply_d755b4().Store(prevent_dce, 0u, 64u, MatrixLayout::RowMajor);
}

