#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_right_f32_8x8 = Matrix<ComponentType::F32, 8, 8, MatrixUse::B, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
Matrix_right_f32_8x8 tint_subgroup_matrix_scalar_op(Matrix_right_f32_8x8 m, float s) {
  Matrix_right_f32_8x8 v = Matrix_right_f32_8x8::Splat(0.0f);
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

Matrix_right_f32_8x8 subgroupMatrixScalarMultiply_decf0b() {
  Matrix_right_f32_8x8 res = tint_subgroup_matrix_scalar_op(Matrix_right_f32_8x8::Splat(0.0f), 8.0f);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixScalarMultiply_decf0b().Store(prevent_dce, 0u, 32u, MatrixLayout::RowMajor);
}

