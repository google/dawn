#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_result_u8_8x8 = Matrix<ComponentType::U8, 8, 8, MatrixUse::Accumulator, MatrixScope::Wave>;

RWByteAddressBuffer sb_rw : register(u0);
void subgroupMatrixStore_7e4900() {
  Matrix_result_u8_8x8::Splat(0u).Store(sb_rw, 4u, 32u, MatrixLayout::RowMajor);
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixStore_7e4900();
}

