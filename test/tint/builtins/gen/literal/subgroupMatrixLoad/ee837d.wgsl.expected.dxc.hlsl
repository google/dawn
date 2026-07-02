#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_result_f16_8x8 = Matrix<ComponentType::F16, 8, 8, MatrixUse::Accumulator, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
ByteAddressBuffer sb_ro : register(t1);
Matrix_result_f16_8x8 subgroupMatrixLoad_ee837d() {
  Matrix_result_f16_8x8 res = Matrix_result_f16_8x8::Load(sb_ro, 2u, 16u, MatrixLayout::ColMajor);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixLoad_ee837d().Store(prevent_dce, 0u, 16u, MatrixLayout::RowMajor);
}

