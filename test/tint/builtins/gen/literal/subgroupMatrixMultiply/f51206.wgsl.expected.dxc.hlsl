#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_result_i32_8x8 = Matrix<ComponentType::I32, 8, 8, MatrixUse::Accumulator, MatrixScope::Wave>;
using Matrix_left_u32_8x8 = Matrix<ComponentType::U32, 8, 8, MatrixUse::A, MatrixScope::Wave>;
using Matrix_right_u32_8x8 = Matrix<ComponentType::U32, 8, 8, MatrixUse::B, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
Matrix_result_i32_8x8 subgroupMatrixMultiply_f51206() {
  Matrix_result_i32_8x8 res = Multiply<ComponentType::I32>(Matrix_left_u32_8x8::Splat(0u), Matrix_right_u32_8x8::Splat(0u));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixMultiply_f51206().Store(prevent_dce, 0u, 32u, MatrixLayout::RowMajor);
}

