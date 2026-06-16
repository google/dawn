#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_left_f16_8x8 = Matrix<ComponentType::F16, 8, 8, MatrixUse::A, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
RWByteAddressBuffer sb_rw : register(u1);
Matrix_left_f16_8x8 subgroupMatrixLoad_ea005f() {
  Matrix_left_f16_8x8 res = Matrix_left_f16_8x8::Load(sb_rw, 2u, 16u, MatrixLayout::ColMajor);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixLoad_ea005f().Store(prevent_dce, 0u, 128u, MatrixLayout::RowMajor);
}

