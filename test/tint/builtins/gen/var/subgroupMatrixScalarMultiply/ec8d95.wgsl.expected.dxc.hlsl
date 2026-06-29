#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_result_f16_8x8 = Matrix<ComponentType::F16, 8, 8, MatrixUse::Accumulator, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
Matrix_result_f16_8x8 tint_subgroup_matrix_scalar_op(Matrix_result_f16_8x8 m, float16_t s) {
  Matrix_result_f16_8x8 v = Matrix_result_f16_8x8::Splat(float16_t(0.0h));
  {
    uint v_1 = 0u;
    v_1 = 0u;
    while(true) {
      uint v_2 = v_1;
      if ((v_2 >= 64u)) {
        break;
      }
      v.Set(v_2, (m.Get(v_2) * s));
      {
        v_1 = (v_2 + 1u);
      }
    }
  }
  return v;
}

Matrix_result_f16_8x8 subgroupMatrixScalarMultiply_ec8d95() {
  Matrix_result_f16_8x8 arg_0 = Matrix_result_f16_8x8::Splat(float16_t(0.0h));
  float16_t arg_1 = float16_t(8.0h);
  Matrix_result_f16_8x8 res = tint_subgroup_matrix_scalar_op(arg_0, arg_1);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixScalarMultiply_ec8d95().Store(prevent_dce, 0u, 16u, MatrixLayout::RowMajor);
}

