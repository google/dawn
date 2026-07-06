#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_left_u32_8x8 = Matrix<ComponentType::U32, 8, 8, MatrixUse::A, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
ByteAddressBuffer sb_ro : register(t1);
Matrix_left_u32_8x8 subgroupMatrixLoad_137d05() {
  int arg_1 = int(1);
  uint arg_2 = 8u;
  uint v = max(arg_2, 8u);
  uint v_1 = asuint(arg_1);
  bool v_2 = (((v_1 + (v * 7u)) + 8u) <= 1024u);
  Matrix_left_u32_8x8 res = Matrix_left_u32_8x8::Load(sb_ro, (0u + (select(v_2, v_1, 0u) * 4u)), (select(v_2, v, 8u) * 4u), MatrixLayout::RowMajor);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixLoad_137d05().Store(prevent_dce, 0u, 32u, MatrixLayout::RowMajor);
}

