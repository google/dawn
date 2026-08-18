#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_right_u8_8x8 = Matrix<ComponentType::U8, 8, 8, MatrixUse::B, MatrixScope::Wave>;
using Matrix_right_u32_8x8 = Matrix<ComponentType::U32, 8, 8, MatrixUse::B, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
Matrix_right_u8_8x8 tint_subgroup_matrix_scalar_op(Matrix_right_u8_8x8 m, uint s) {
  Matrix_right_u32_8x8 result = m.Cast<ComponentType::U32>();
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
  return result.Cast<ComponentType::U8>();
}

Matrix_right_u8_8x8 subgroupMatrixScalarSubtract_b49213() {
  Matrix_right_u8_8x8 res = tint_subgroup_matrix_scalar_op(Matrix_right_u8_8x8::Splat(0u), 8u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixScalarSubtract_b49213().Store(prevent_dce, 0u, 64u, MatrixLayout::RowMajor);
}

