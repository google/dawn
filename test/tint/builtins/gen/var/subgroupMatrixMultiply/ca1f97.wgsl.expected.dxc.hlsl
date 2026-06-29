#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_result_i32_8x8 = Matrix<ComponentType::I32, 8, 8, MatrixUse::Accumulator, MatrixScope::Wave>;
using Matrix_left_i8_8x8 = Matrix<ComponentType::I8, 8, 8, MatrixUse::A, MatrixScope::Wave>;
using Matrix_right_i8_8x8 = Matrix<ComponentType::I8, 8, 8, MatrixUse::B, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
Matrix_result_i32_8x8 subgroupMatrixMultiply_ca1f97() {
  Matrix_left_i8_8x8 arg_0 = Matrix_left_i8_8x8::Splat(int(0));
  Matrix_right_i8_8x8 arg_1 = Matrix_right_i8_8x8::Splat(int(0));
  Matrix_result_i32_8x8 res = Multiply<ComponentType::I32>(arg_0, arg_1);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixMultiply_ca1f97().Store(prevent_dce, 0u, 32u, MatrixLayout::RowMajor);
}

