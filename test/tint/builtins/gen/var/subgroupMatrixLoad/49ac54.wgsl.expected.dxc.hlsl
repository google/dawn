#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_result_f16_8x8 = Matrix<ComponentType::F16, 8, 8, MatrixUse::Accumulator, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
ByteAddressBuffer sb_ro : register(t1);
Matrix_result_f16_8x8 subgroupMatrixLoad_49ac54() {
  int arg_1 = int(1);
  int arg_2 = int(8);
  uint v = max(asuint(arg_2), 8u);
  uint v_1 = asuint(arg_1);
  bool v_2 = (((v_1 + (v * 7u)) + 8u) <= 1024u);
  Matrix_result_f16_8x8 res = Matrix_result_f16_8x8::Load(sb_ro, (0u + (select(v_2, v_1, 0u) * 2u)), (select(v_2, v, 8u) * 2u), MatrixLayout::RowMajor);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixLoad_49ac54().Store(prevent_dce, 0u, 16u, MatrixLayout::RowMajor);
}

