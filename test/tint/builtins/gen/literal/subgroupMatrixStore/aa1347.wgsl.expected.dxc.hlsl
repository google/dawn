#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_left_f32_8x8 = Matrix<ComponentType::F32, 8, 8, MatrixUse::A, MatrixScope::Wave>;

RWByteAddressBuffer sb_rw : register(u0);
void subgroupMatrixStore_aa1347() {
  Matrix_left_f32_8x8::Splat(0.0f).Store(sb_rw, 4u, 32u, MatrixLayout::RowMajor);
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixStore_aa1347();
}

