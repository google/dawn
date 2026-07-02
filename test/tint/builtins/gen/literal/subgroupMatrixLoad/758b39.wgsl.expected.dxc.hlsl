#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_left_i32_8x8 = Matrix<ComponentType::I32, 8, 8, MatrixUse::A, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
ByteAddressBuffer sb_ro : register(t1);
Matrix_left_i32_8x8 subgroupMatrixLoad_758b39() {
  Matrix_left_i32_8x8 res = Matrix_left_i32_8x8::Load(sb_ro, 4u, 32u, MatrixLayout::RowMajor);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixLoad_758b39().Store(prevent_dce, 0u, 32u, MatrixLayout::RowMajor);
}

