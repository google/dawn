#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_left_u32_8x8 = Matrix<ComponentType::U32, 8, 8, MatrixUse::A, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
Matrix_left_u32_8x8 tint_subgroup_matrix_scalar_op(Matrix_left_u32_8x8 m, uint s) {
  Matrix_left_u32_8x8 v = Matrix_left_u32_8x8::Splat(0u);
  {
    uint v_1 = 0u;
    v_1 = 0u;
    while(true) {
      uint v_2 = v_1;
      if ((v_2 >= 64u)) {
        break;
      }
      v.Set(v_2, (m.Get(v_2) + s));
      {
        v_1 = (v_2 + 1u);
      }
    }
  }
  return v;
}

Matrix_left_u32_8x8 subgroupMatrixScalarAdd_24b324() {
  Matrix_left_u32_8x8 arg_0 = Matrix_left_u32_8x8::Splat(0u);
  uint arg_1 = 8u;
  Matrix_left_u32_8x8 res = tint_subgroup_matrix_scalar_op(arg_0, arg_1);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixScalarAdd_24b324().Store(prevent_dce, 0u, 32u, MatrixLayout::RowMajor);
}

