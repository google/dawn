#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_right_i8_8x8 = Matrix<ComponentType::I8, 8, 8, MatrixUse::B, MatrixScope::Wave>;
using Matrix_right_i32_8x8 = Matrix<ComponentType::I32, 8, 8, MatrixUse::B, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
Matrix_right_i8_8x8 tint_subgroup_matrix_scalar_op(Matrix_right_i8_8x8 m, int s) {
  Matrix_right_i32_8x8 result = m.Cast<ComponentType::I32>();
  {
    uint v = 0u;
    v = 0u;
    while(true) {
      uint v_1 = v;
      if ((v_1 >= result.Length())) {
        break;
      }
      result.Set(v_1, (result.Get(v_1) + s));
      {
        v = (v_1 + 1u);
      }
    }
  }
  return result.Cast<ComponentType::I8>();
}

Matrix_right_i8_8x8 subgroupMatrixScalarAdd_788efe() {
  Matrix_right_i8_8x8 arg_0 = Matrix_right_i8_8x8::Splat(int(0));
  int arg_1 = int(8);
  Matrix_right_i8_8x8 res = tint_subgroup_matrix_scalar_op(arg_0, arg_1);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixScalarAdd_788efe().Store(prevent_dce, 0u, 64u, MatrixLayout::RowMajor);
}

