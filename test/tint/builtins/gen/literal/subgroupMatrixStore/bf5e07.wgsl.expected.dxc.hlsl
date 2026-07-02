#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_result_f16_8x8 = Matrix<ComponentType::F16, 8, 8, MatrixUse::Accumulator, MatrixScope::Wave>;

RWByteAddressBuffer sb_rw : register(u0);
void subgroupMatrixStore_bf5e07() {
  Matrix_result_f16_8x8::Splat(float16_t(0.0h)).Store(sb_rw, 2u, 16u, MatrixLayout::ColMajor);
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixStore_bf5e07();
}

