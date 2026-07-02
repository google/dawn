#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_result_u8_8x8 = Matrix<ComponentType::U8, 8, 8, MatrixUse::Accumulator, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
RWByteAddressBuffer sb_rw : register(u1);
Matrix_result_u8_8x8 subgroupMatrixLoad_a42526() {
  Matrix_result_u8_8x8 res = Matrix_result_u8_8x8::Load(sb_rw, 4u, 32u, MatrixLayout::RowMajor);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixLoad_a42526().Store(prevent_dce, 0u, 8u, MatrixLayout::RowMajor);
}

