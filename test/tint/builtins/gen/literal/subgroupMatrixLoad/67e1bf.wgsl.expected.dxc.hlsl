#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_result_f32_8x8 = Matrix<ComponentType::F32, 8, 8, MatrixUse::Accumulator, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
ByteAddressBuffer sb_ro : register(t1);
Matrix_result_f32_8x8 subgroupMatrixLoad_67e1bf() {
  Matrix_result_f32_8x8 res = Matrix_result_f32_8x8::Load(sb_ro, 4u, 32u, MatrixLayout::ColMajor);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixLoad_67e1bf().Store(prevent_dce, 0u, 32u, MatrixLayout::RowMajor);
}

