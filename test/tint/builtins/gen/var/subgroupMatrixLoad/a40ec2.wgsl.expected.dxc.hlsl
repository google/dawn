#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_left_f16_8x8 = Matrix<ComponentType::F16, 8, 8, MatrixUse::A, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
ByteAddressBuffer sb_ro : register(t1);
Matrix_left_f16_8x8 subgroupMatrixLoad_a40ec2() {
  uint arg_1 = 1u;
  int arg_2 = int(8);
  uint v = arg_1;
  uint v_1 = max(asuint(arg_2), 8u);
  bool v_2 = (((v + (v_1 * 7u)) + 8u) <= 1024u);
  Matrix_left_f16_8x8 res = Matrix_left_f16_8x8::Load(sb_ro, (0u + (select(v_2, v, 0u) * 2u)), (select(v_2, v_1, 8u) * 2u), MatrixLayout::RowMajor);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixLoad_a40ec2().Store(prevent_dce, 0u, 16u, MatrixLayout::RowMajor);
}

