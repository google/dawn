#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_right_f32_8x8 = Matrix<ComponentType::F32, 8, 8, MatrixUse::B, MatrixScope::Wave>;

RWByteAddressBuffer sb_rw : register(u0);
void subgroupMatrixStore_a3c63e() {
  Matrix_right_f32_8x8::Splat(0.0f).Store(sb_rw, 4u, 32u, MatrixLayout::RowMajor);
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixStore_a3c63e();
}

