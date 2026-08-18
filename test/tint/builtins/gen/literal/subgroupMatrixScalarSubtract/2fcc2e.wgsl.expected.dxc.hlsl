#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_left_i8_8x8 = Matrix<ComponentType::I8, 8, 8, MatrixUse::A, MatrixScope::Wave>;
using Matrix_left_i32_8x8 = Matrix<ComponentType::I32, 8, 8, MatrixUse::A, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
Matrix_left_i8_8x8 tint_subgroup_matrix_scalar_op(Matrix_left_i8_8x8 m, int s) {
  Matrix_left_i32_8x8 result = m.Cast<ComponentType::I32>();
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
  return result.Cast<ComponentType::I8>();
}

Matrix_left_i8_8x8 subgroupMatrixScalarSubtract_2fcc2e() {
  Matrix_left_i8_8x8 res = tint_subgroup_matrix_scalar_op(Matrix_left_i8_8x8::Splat(int(0)), int(8));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixScalarSubtract_2fcc2e().Store(prevent_dce, 0u, 64u, MatrixLayout::RowMajor);
}

