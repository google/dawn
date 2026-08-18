#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_result_i8_8x8 = Matrix<ComponentType::I8, 8, 8, MatrixUse::Accumulator, MatrixScope::Wave>;
using Matrix_result_i32_8x8 = Matrix<ComponentType::I32, 8, 8, MatrixUse::Accumulator, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
Matrix_result_i8_8x8 tint_subgroup_matrix_scalar_op(Matrix_result_i8_8x8 m, int s) {
  Matrix_result_i32_8x8 result = m.Cast<ComponentType::I32>();
  {
    uint v = 0u;
    v = 0u;
    while(true) {
      uint v_1 = v;
      if ((v_1 >= result.Length())) {
        break;
      }
      result.Set(v_1, (result.Get(v_1) * s));
      {
        v = (v_1 + 1u);
      }
    }
  }
  return result.Cast<ComponentType::I8>();
}

Matrix_result_i8_8x8 subgroupMatrixScalarMultiply_38f728() {
  Matrix_result_i8_8x8 res = tint_subgroup_matrix_scalar_op(Matrix_result_i8_8x8::Splat(int(0)), int(8));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixScalarMultiply_38f728().Store(prevent_dce, 0u, 64u, MatrixLayout::RowMajor);
}

