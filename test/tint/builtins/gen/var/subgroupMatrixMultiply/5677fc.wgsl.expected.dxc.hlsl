#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_result_f32_8x8 = Matrix<ComponentType::F32, 8, 8, MatrixUse::Accumulator, MatrixScope::Wave>;
using Matrix_left_f16_8x8 = Matrix<ComponentType::F16, 8, 8, MatrixUse::A, MatrixScope::Wave>;
using Matrix_right_f16_8x8 = Matrix<ComponentType::F16, 8, 8, MatrixUse::B, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
Matrix_result_f32_8x8 subgroupMatrixMultiply_5677fc() {
  Matrix_left_f16_8x8 arg_0 = Matrix_left_f16_8x8::Splat(float16_t(0.0h));
  Matrix_right_f16_8x8 arg_1 = Matrix_right_f16_8x8::Splat(float16_t(0.0h));
  Matrix_result_f32_8x8 res = Multiply<ComponentType::F32>(arg_0, arg_1);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixMultiply_5677fc().Store(prevent_dce, 0u, 32u, MatrixLayout::RowMajor);
}

