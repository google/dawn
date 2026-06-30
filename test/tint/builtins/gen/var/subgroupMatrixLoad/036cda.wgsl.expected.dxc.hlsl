#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_left_u8_8x8 = Matrix<ComponentType::U8, 8, 8, MatrixUse::A, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
ByteAddressBuffer sb_ro : register(t1);
Matrix_left_u8_8x8 subgroupMatrixLoad_036cda() {
  uint arg_1 = 1u;
  uint arg_2 = 8u;
  uint v = arg_1;
  uint v_1 = max(arg_2, 2u);
  Matrix_left_u8_8x8 v_2 = Matrix_left_u8_8x8::Splat(0u);
  if ((((v + (v_1 * 7u)) + 2u) <= 1024u)) {
    v_2 = Matrix_left_u8_8x8::Load(sb_ro, (0u + (v * 4u)), (v_1 * 4u), MatrixLayout::RowMajor);
  }
  Matrix_left_u8_8x8 res = v_2;
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixLoad_036cda().Store(prevent_dce, 0u, 8u, MatrixLayout::RowMajor);
}

