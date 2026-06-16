#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_left_i8_8x8 = Matrix<ComponentType::I8, 8, 8, MatrixUse::A, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
ByteAddressBuffer sb_ro : register(t1);
Matrix_left_i8_8x8 subgroupMatrixLoad_384123() {
  Matrix_left_i8_8x8 res = Matrix_left_i8_8x8::Load(sb_ro, 1u, 8u, MatrixLayout::ColMajor);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixLoad_384123().Store(prevent_dce, 0u, 64u, MatrixLayout::RowMajor);
}

