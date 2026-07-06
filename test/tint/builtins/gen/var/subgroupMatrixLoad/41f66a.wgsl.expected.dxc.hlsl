#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_result_i32_8x8 = Matrix<ComponentType::I32, 8, 8, MatrixUse::Accumulator, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
ByteAddressBuffer sb_ro : register(t1);
Matrix_result_i32_8x8 subgroupMatrixLoad_41f66a() {
  int arg_1 = int(1);
  uint arg_2 = 8u;
  uint v = max(arg_2, 8u);
  uint v_1 = asuint(arg_1);
  bool v_2 = (((v_1 + (v * 7u)) + 8u) <= 1024u);
  Matrix_result_i32_8x8 res = Matrix_result_i32_8x8::Load(sb_ro, (0u + (select(v_2, v_1, 0u) * 4u)), (select(v_2, v, 8u) * 4u), MatrixLayout::ColMajor);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixLoad_41f66a().Store(prevent_dce, 0u, 32u, MatrixLayout::RowMajor);
}

