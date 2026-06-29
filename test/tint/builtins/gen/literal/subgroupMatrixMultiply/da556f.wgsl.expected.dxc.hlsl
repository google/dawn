#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_result_u32_8x8 = Matrix<ComponentType::U32, 8, 8, MatrixUse::Accumulator, MatrixScope::Wave>;
using Matrix_left_u32_8x8 = Matrix<ComponentType::U32, 8, 8, MatrixUse::A, MatrixScope::Wave>;
using Matrix_right_u32_8x8 = Matrix<ComponentType::U32, 8, 8, MatrixUse::B, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
Matrix_result_u32_8x8 subgroupMatrixMultiply_da556f() {
  Matrix_result_u32_8x8 res = Multiply(Matrix_left_u32_8x8::Splat(0u), Matrix_right_u32_8x8::Splat(0u));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixMultiply_da556f().Store(prevent_dce, 0u, 32u, MatrixLayout::RowMajor);
}

