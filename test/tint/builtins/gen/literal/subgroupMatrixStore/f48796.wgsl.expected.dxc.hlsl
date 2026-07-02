#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_right_i8_8x8 = Matrix<ComponentType::I8, 8, 8, MatrixUse::B, MatrixScope::Wave>;

RWByteAddressBuffer sb_rw : register(u0);
void subgroupMatrixStore_f48796() {
  Matrix_right_i8_8x8::Splat(int(0)).Store(sb_rw, 4u, 32u, MatrixLayout::ColMajor);
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixStore_f48796();
}

