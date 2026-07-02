#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_right_u32_8x8 = Matrix<ComponentType::U32, 8, 8, MatrixUse::B, MatrixScope::Wave>;

RWByteAddressBuffer sb_rw : register(u0);
void subgroupMatrixStore_46213f() {
  Matrix_right_u32_8x8::Splat(0u).Store(sb_rw, 4u, 32u, MatrixLayout::ColMajor);
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixStore_46213f();
}

