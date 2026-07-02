#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_right_i32_8x8 = Matrix<ComponentType::I32, 8, 8, MatrixUse::B, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
RWByteAddressBuffer sb_rw : register(u1);
Matrix_right_i32_8x8 subgroupMatrixLoad_023b36() {
  Matrix_right_i32_8x8 res = Matrix_right_i32_8x8::Load(sb_rw, 4u, 32u, MatrixLayout::RowMajor);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixLoad_023b36().Store(prevent_dce, 0u, 32u, MatrixLayout::RowMajor);
}

