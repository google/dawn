#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_right_i8_8x8 = Matrix<ComponentType::I8, 8, 8, MatrixUse::B, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
RWByteAddressBuffer sb_rw : register(u1);
Matrix_right_i8_8x8 subgroupMatrixLoad_b7c4ff() {
  Matrix_right_i8_8x8 res = Matrix_right_i8_8x8::Load(sb_rw, 1u, 8u, MatrixLayout::ColMajor);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixLoad_b7c4ff().Store(prevent_dce, 0u, 8u, MatrixLayout::RowMajor);
}

