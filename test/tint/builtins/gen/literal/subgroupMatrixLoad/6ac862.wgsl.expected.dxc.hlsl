#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_left_f32_8x8 = Matrix<ComponentType::F32, 8, 8, MatrixUse::A, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
RWByteAddressBuffer sb_rw : register(u1);
Matrix_left_f32_8x8 subgroupMatrixLoad_6ac862() {
  Matrix_left_f32_8x8 res = Matrix_left_f32_8x8::Load(sb_rw, 4u, 32u, MatrixLayout::ColMajor);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixLoad_6ac862().Store(prevent_dce, 0u, 256u, MatrixLayout::RowMajor);
}

