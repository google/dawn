#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_result_u8_8x8 = Matrix<ComponentType::U8, 8, 8, MatrixUse::Accumulator, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
ByteAddressBuffer sb_ro : register(t1);
Matrix_result_u8_8x8 subgroupMatrixLoad_22a971() {
  uint arg_1 = 1u;
  int arg_2 = int(8);
  uint v = arg_1;
  uint v_1 = max(asuint(arg_2), 2u);
  bool v_2 = (((v + (v_1 * 7u)) + 2u) <= 1024u);
  Matrix_result_u8_8x8 res = Matrix_result_u8_8x8::Load(sb_ro, (0u + (select(v_2, v, 0u) * 4u)), (select(v_2, v_1, 2u) * 4u), MatrixLayout::ColMajor);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixLoad_22a971().Store(prevent_dce, 0u, 8u, MatrixLayout::RowMajor);
}

