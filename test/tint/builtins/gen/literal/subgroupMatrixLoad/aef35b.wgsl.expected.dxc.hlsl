#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_result_i8_8x8 = Matrix<ComponentType::I8, 8, 8, MatrixUse::Accumulator, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
RWByteAddressBuffer sb_rw : register(u1);
Matrix_result_i8_8x8 subgroupMatrixLoad_aef35b() {
  Matrix_result_i8_8x8 res = Matrix_result_i8_8x8::Load(sb_rw, 1u, 8u, MatrixLayout::RowMajor);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixLoad_aef35b().Store(prevent_dce, 0u, 64u, MatrixLayout::RowMajor);
}

