#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_right_f16_8x8 = Matrix<ComponentType::F16, 8, 8, MatrixUse::B, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
RWByteAddressBuffer sb_rw : register(u1);
Matrix_right_f16_8x8 subgroupMatrixLoad_16347b() {
  Matrix_right_f16_8x8 res = Matrix_right_f16_8x8::Load(sb_rw, 2u, 16u, MatrixLayout::ColMajor);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixLoad_16347b().Store(prevent_dce, 0u, 16u, MatrixLayout::RowMajor);
}

