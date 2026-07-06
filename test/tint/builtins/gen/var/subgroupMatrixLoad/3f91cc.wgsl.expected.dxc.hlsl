#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_right_f16_8x8 = Matrix<ComponentType::F16, 8, 8, MatrixUse::B, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
ByteAddressBuffer sb_ro : register(t1);
Matrix_right_f16_8x8 subgroupMatrixLoad_3f91cc() {
  int arg_1 = int(1);
  uint arg_2 = 8u;
  uint v = max(arg_2, 8u);
  uint v_1 = asuint(arg_1);
  bool v_2 = (((v_1 + (v * 7u)) + 8u) <= 1024u);
  Matrix_right_f16_8x8 res = Matrix_right_f16_8x8::Load(sb_ro, (0u + (select(v_2, v_1, 0u) * 2u)), (select(v_2, v, 8u) * 2u), MatrixLayout::RowMajor);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixLoad_3f91cc().Store(prevent_dce, 0u, 16u, MatrixLayout::RowMajor);
}

