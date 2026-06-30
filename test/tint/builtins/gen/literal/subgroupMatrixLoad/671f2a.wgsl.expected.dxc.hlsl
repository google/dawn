#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_left_u8_8x8 = Matrix<ComponentType::U8, 8, 8, MatrixUse::A, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
RWByteAddressBuffer sb_rw : register(u1);
Matrix_left_u8_8x8 subgroupMatrixLoad_671f2a() {
  Matrix_left_u8_8x8 res = Matrix_left_u8_8x8::Load(sb_rw, 4u, 32u, MatrixLayout::RowMajor);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixLoad_671f2a().Store(prevent_dce, 0u, 8u, MatrixLayout::RowMajor);
}

