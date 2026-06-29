#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_result_f32_8x8 = Matrix<ComponentType::F32, 8, 8, MatrixUse::Accumulator, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
RWByteAddressBuffer sb_rw : register(u1);
Matrix_result_f32_8x8 subgroupMatrixLoad_7fb60e() {
  Matrix_result_f32_8x8 res = Matrix_result_f32_8x8::Load(sb_rw, 4u, 32u, MatrixLayout::RowMajor);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixLoad_7fb60e().Store(prevent_dce, 0u, 32u, MatrixLayout::RowMajor);
}

