#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_result_i32_8x8 = Matrix<ComponentType::I32, 8, 8, MatrixUse::Accumulator, MatrixScope::Wave>;
using Matrix_left_i32_8x8 = Matrix<ComponentType::I32, 8, 8, MatrixUse::A, MatrixScope::Wave>;
using Matrix_right_i32_8x8 = Matrix<ComponentType::I32, 8, 8, MatrixUse::B, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
Matrix_result_i32_8x8 subgroupMatrixMultiply_2969da() {
  Matrix_result_i32_8x8 res = Multiply(Matrix_left_i32_8x8::Splat(int(0)), Matrix_right_i32_8x8::Splat(int(0)));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixMultiply_2969da().Store(prevent_dce, 0u, 32u, MatrixLayout::RowMajor);
}

