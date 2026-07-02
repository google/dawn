#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_left_i8_8x8 = Matrix<ComponentType::I8, 8, 8, MatrixUse::A, MatrixScope::Wave>;

RWByteAddressBuffer sb_rw : register(u0);
void subgroupMatrixStore_dffe18() {
  Matrix_left_i8_8x8::Splat(int(0)).Store(sb_rw, 4u, 32u, MatrixLayout::RowMajor);
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixStore_dffe18();
}

