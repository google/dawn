#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_right_u8_8x8 = Matrix<ComponentType::U8, 8, 8, MatrixUse::B, MatrixScope::Wave>;

RWByteAddressBuffer sb_rw : register(u0);
void subgroupMatrixStore_1498f5() {
  Matrix_right_u8_8x8::Splat(0u).Store(sb_rw, 1u, 8u, MatrixLayout::RowMajor);
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixStore_1498f5();
}

