#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_right_u8_8x8 = Matrix<ComponentType::U8, 8, 8, MatrixUse::B, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
RWByteAddressBuffer sb_rw : register(u1);
Matrix_right_u8_8x8 subgroupMatrixLoad_d041db() {
  Matrix_right_u8_8x8 res = Matrix_right_u8_8x8::Load(sb_rw, 1u, 8u, MatrixLayout::RowMajor);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixLoad_d041db().Store(prevent_dce, 0u, 64u, MatrixLayout::RowMajor);
}

