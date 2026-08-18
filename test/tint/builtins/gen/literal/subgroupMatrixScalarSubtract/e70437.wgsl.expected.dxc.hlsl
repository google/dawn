#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_right_f32_8x8 = Matrix<ComponentType::F32, 8, 8, MatrixUse::B, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
Matrix_right_f32_8x8 tint_subgroup_matrix_scalar_op(Matrix_right_f32_8x8 m, float s) {
  Matrix_right_f32_8x8 result = m;
  {
    uint v = 0u;
    v = 0u;
    while(true) {
      uint v_1 = v;
      if ((v_1 >= result.Length())) {
        break;
      }
      result.Set(v_1, (result.Get(v_1) - s));
      {
        v = (v_1 + 1u);
      }
    }
  }
  return result;
}

Matrix_right_f32_8x8 subgroupMatrixScalarSubtract_e70437() {
  Matrix_right_f32_8x8 res = tint_subgroup_matrix_scalar_op(Matrix_right_f32_8x8::Splat(0.0f), 8.0f);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixScalarSubtract_e70437().Store(prevent_dce, 0u, 64u, MatrixLayout::RowMajor);
}

