#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_result_i32_8x8 = Matrix<ComponentType::I32, 8, 8, MatrixUse::Accumulator, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
RWByteAddressBuffer sb_rw : register(u1);
Matrix_result_i32_8x8 subgroupMatrixLoad_e6a972() {
  int arg_1 = int(1);
  int arg_2 = int(8);
  uint v = max(asuint(arg_2), 8u);
  uint v_1 = asuint(arg_1);
  bool v_2 = (((v_1 + (v * 7u)) + 8u) <= 1024u);
  Matrix_result_i32_8x8 res = Matrix_result_i32_8x8::Load(sb_rw, (0u + (select(v_2, v_1, 0u) * 4u)), (select(v_2, v, 8u) * 4u), MatrixLayout::ColMajor);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixLoad_e6a972().Store(prevent_dce, 0u, 32u, MatrixLayout::RowMajor);
}

