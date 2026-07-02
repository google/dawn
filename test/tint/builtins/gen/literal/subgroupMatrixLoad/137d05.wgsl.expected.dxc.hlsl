#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_left_u32_8x8 = Matrix<ComponentType::U32, 8, 8, MatrixUse::A, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
ByteAddressBuffer sb_ro : register(t1);
Matrix_left_u32_8x8 subgroupMatrixLoad_137d05() {
  Matrix_left_u32_8x8 res = Matrix_left_u32_8x8::Load(sb_ro, 4u, 32u, MatrixLayout::RowMajor);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixLoad_137d05().Store(prevent_dce, 0u, 32u, MatrixLayout::RowMajor);
}

