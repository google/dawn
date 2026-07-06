#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_right_i8_8x8 = Matrix<ComponentType::I8, 8, 8, MatrixUse::B, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
ByteAddressBuffer sb_ro : register(t1);
Matrix_right_i8_8x8 subgroupMatrixLoad_e03b14() {
  int arg_1 = int(1);
  uint arg_2 = 8u;
  uint v = max(arg_2, 2u);
  uint v_1 = asuint(arg_1);
  bool v_2 = (((v_1 + (v * 7u)) + 2u) <= 1024u);
  Matrix_right_i8_8x8 res = Matrix_right_i8_8x8::Load(sb_ro, (0u + (select(v_2, v_1, 0u) * 4u)), (select(v_2, v, 2u) * 4u), MatrixLayout::RowMajor);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixLoad_e03b14().Store(prevent_dce, 0u, 8u, MatrixLayout::RowMajor);
}

