#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_right_u32_8x8 = Matrix<ComponentType::U32, 8, 8, MatrixUse::B, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
ByteAddressBuffer sb_ro : register(t1);
Matrix_right_u32_8x8 subgroupMatrixLoad_d05fc6() {
  uint arg_1 = 1u;
  uint arg_2 = 8u;
  uint v = arg_1;
  uint v_1 = max(arg_2, 8u);
  bool v_2 = (((v + (v_1 * 7u)) + 8u) <= 1024u);
  Matrix_right_u32_8x8 res = Matrix_right_u32_8x8::Load(sb_ro, (0u + (select(v_2, v, 0u) * 4u)), (select(v_2, v_1, 8u) * 4u), MatrixLayout::ColMajor);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixLoad_d05fc6().Store(prevent_dce, 0u, 32u, MatrixLayout::RowMajor);
}

