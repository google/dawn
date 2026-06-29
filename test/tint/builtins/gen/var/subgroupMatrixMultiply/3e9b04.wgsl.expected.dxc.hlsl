#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_result_u32_8x8 = Matrix<ComponentType::U32, 8, 8, MatrixUse::Accumulator, MatrixScope::Wave>;
using Matrix_left_i32_8x8 = Matrix<ComponentType::I32, 8, 8, MatrixUse::A, MatrixScope::Wave>;
using Matrix_right_i32_8x8 = Matrix<ComponentType::I32, 8, 8, MatrixUse::B, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
Matrix_result_u32_8x8 subgroupMatrixMultiply_3e9b04() {
  Matrix_left_i32_8x8 arg_0 = Matrix_left_i32_8x8::Splat(int(0));
  Matrix_right_i32_8x8 arg_1 = Matrix_right_i32_8x8::Splat(int(0));
  Matrix_result_u32_8x8 res = Multiply<ComponentType::U32>(arg_0, arg_1);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixMultiply_3e9b04().Store(prevent_dce, 0u, 32u, MatrixLayout::RowMajor);
}

