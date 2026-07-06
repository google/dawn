#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_result_i8_8x8 = Matrix<ComponentType::I8, 8, 8, MatrixUse::Accumulator, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
ByteAddressBuffer sb_ro : register(t1);
Matrix_result_i8_8x8 subgroupMatrixLoad_c2d89e() {
  int arg_1 = int(1);
  int arg_2 = int(8);
  uint v = max(asuint(arg_2), 2u);
  uint v_1 = asuint(arg_1);
  bool v_2 = (((v_1 + (v * 7u)) + 2u) <= 1024u);
  Matrix_result_i8_8x8 res = Matrix_result_i8_8x8::Load(sb_ro, (0u + (select(v_2, v_1, 0u) * 4u)), (select(v_2, v, 2u) * 4u), MatrixLayout::RowMajor);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixLoad_c2d89e().Store(prevent_dce, 0u, 8u, MatrixLayout::RowMajor);
}

