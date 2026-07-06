#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_right_f16_8x8 = Matrix<ComponentType::F16, 8, 8, MatrixUse::B, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
RWByteAddressBuffer sb_rw : register(u1);
Matrix_right_f16_8x8 subgroupMatrixLoad_359913() {
  uint arg_1 = 1u;
  uint arg_2 = 8u;
  uint v = arg_1;
  uint v_1 = max(arg_2, 8u);
  bool v_2 = (((v + (v_1 * 7u)) + 8u) <= 1024u);
  Matrix_right_f16_8x8 res = Matrix_right_f16_8x8::Load(sb_rw, (0u + (select(v_2, v, 0u) * 2u)), (select(v_2, v_1, 8u) * 2u), MatrixLayout::RowMajor);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixLoad_359913().Store(prevent_dce, 0u, 16u, MatrixLayout::RowMajor);
}

