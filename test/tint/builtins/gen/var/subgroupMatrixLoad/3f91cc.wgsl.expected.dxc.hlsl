#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_right_f16_8x8 = Matrix<ComponentType::F16, 8, 8, MatrixUse::B, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
ByteAddressBuffer sb_ro : register(t1);
Matrix_right_f16_8x8 subgroupMatrixLoad_3f91cc() {
  int arg_1 = int(1);
  uint arg_2 = 8u;
  int v = arg_1;
  uint v_1 = max(arg_2, 8u);
  Matrix_right_f16_8x8 v_2 = Matrix_right_f16_8x8::Splat(float16_t(0.0h));
  if ((((asuint(v) + (v_1 * 7u)) + 8u) <= 1024u)) {
    v_2 = Matrix_right_f16_8x8::Load(sb_ro, (0u + (uint(v) * 2u)), (v_1 * 2u), MatrixLayout::RowMajor);
  }
  Matrix_right_f16_8x8 res = v_2;
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixLoad_3f91cc().Store(prevent_dce, 0u, 16u, MatrixLayout::RowMajor);
}

