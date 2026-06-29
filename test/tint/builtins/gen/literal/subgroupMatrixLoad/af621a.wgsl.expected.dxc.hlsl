#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_left_u8_8x8 = Matrix<ComponentType::U8, 8, 8, MatrixUse::A, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
ByteAddressBuffer sb_ro : register(t1);
Matrix_left_u8_8x8 subgroupMatrixLoad_af621a() {
  Matrix_left_u8_8x8 res = Matrix_left_u8_8x8::Load(sb_ro, 1u, 8u, MatrixLayout::ColMajor);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixLoad_af621a().Store(prevent_dce, 0u, 8u, MatrixLayout::RowMajor);
}

