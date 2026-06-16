#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_result_u8_8x8 = Matrix<ComponentType::U8, 8, 8, MatrixUse::Accumulator, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
ByteAddressBuffer sb_ro : register(t1);
Matrix_result_u8_8x8 subgroupMatrixLoad_bd02c0() {
  Matrix_result_u8_8x8 res = Matrix_result_u8_8x8::Load(sb_ro, 1u, 8u, MatrixLayout::ColMajor);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixLoad_bd02c0().Store(prevent_dce, 0u, 64u, MatrixLayout::RowMajor);
}

