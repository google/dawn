#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_result_i8_8x8 = Matrix<ComponentType::I8, 8, 8, MatrixUse::Accumulator, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
RWByteAddressBuffer sb_rw : register(u1);
Matrix_result_i8_8x8 subgroupMatrixLoad_aef35b() {
  uint arg_1 = 1u;
  uint arg_2 = 8u;
  uint v = arg_1;
  uint v_1 = max(arg_2, 2u);
  bool v_2 = (((v + (v_1 * 7u)) + 2u) <= 1024u);
  Matrix_result_i8_8x8 res = Matrix_result_i8_8x8::Load(sb_rw, (0u + (select(v_2, v, 0u) * 4u)), (select(v_2, v_1, 2u) * 4u), MatrixLayout::RowMajor);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixLoad_aef35b().Store(prevent_dce, 0u, 8u, MatrixLayout::RowMajor);
}

