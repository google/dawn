#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_right_u8_8x8 = Matrix<ComponentType::U8, 8, 8, MatrixUse::B, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
RWByteAddressBuffer sb_rw : register(u1);
Matrix_right_u8_8x8 subgroupMatrixLoad_89067e() {
  int arg_1 = int(1);
  uint arg_2 = 8u;
  int v = arg_1;
  uint v_1 = max(arg_2, 2u);
  Matrix_right_u8_8x8 v_2 = Matrix_right_u8_8x8::Splat(0u);
  if ((((asuint(v) + (v_1 * 7u)) + 2u) <= 1024u)) {
    v_2 = Matrix_right_u8_8x8::Load(sb_rw, (0u + (uint(v) * 4u)), (v_1 * 4u), MatrixLayout::ColMajor);
  }
  Matrix_right_u8_8x8 res = v_2;
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixLoad_89067e().Store(prevent_dce, 0u, 8u, MatrixLayout::RowMajor);
}

