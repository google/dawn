#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_right_i8_8x8 = Matrix<ComponentType::I8, 8, 8, MatrixUse::B, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
ByteAddressBuffer sb_ro : register(t1);
Matrix_right_i8_8x8 subgroupMatrixLoad_a4110d() {
  Matrix_right_i8_8x8 res = Matrix_right_i8_8x8::Load(sb_ro, 1u, 8u, MatrixLayout::ColMajor);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixLoad_a4110d().Store(prevent_dce, 0u, 8u, MatrixLayout::RowMajor);
}

