#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_right_i32_8x8 = Matrix<ComponentType::I32, 8, 8, MatrixUse::B, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
ByteAddressBuffer sb_ro : register(t1);
Matrix_right_i32_8x8 subgroupMatrixLoad_63cf94() {
  Matrix_right_i32_8x8 res = Matrix_right_i32_8x8::Load(sb_ro, 4u, 32u, MatrixLayout::ColMajor);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixLoad_63cf94().Store(prevent_dce, 0u, 256u, MatrixLayout::RowMajor);
}

