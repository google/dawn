#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_right_u8_8x8 = Matrix<ComponentType::U8, 8, 8, MatrixUse::B, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
ByteAddressBuffer sb_ro : register(t1);
Matrix_right_u8_8x8 subgroupMatrixLoad_99d9fc() {
  Matrix_right_u8_8x8 res = Matrix_right_u8_8x8::Load(sb_ro, 1u, 8u, MatrixLayout::ColMajor);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixLoad_99d9fc().Store(prevent_dce, 0u, 8u, MatrixLayout::RowMajor);
}

