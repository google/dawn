#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_right_f16_8x8 = Matrix<ComponentType::F16, 8, 8, MatrixUse::B, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
RWByteAddressBuffer sb_rw : register(u1);
Matrix_right_f16_8x8 subgroupMatrixLoad_7ef548() {
  uint arg_1 = 1u;
  int arg_2 = int(8);
  uint v = arg_1;
  uint v_1 = max(asuint(arg_2), 8u);
  Matrix_right_f16_8x8 v_2 = Matrix_right_f16_8x8::Splat(float16_t(0.0h));
  if ((((v + (v_1 * 7u)) + 8u) <= 1024u)) {
    v_2 = Matrix_right_f16_8x8::Load(sb_rw, (0u + (v * 2u)), (v_1 * 2u), MatrixLayout::ColMajor);
  }
  Matrix_right_f16_8x8 res = v_2;
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixLoad_7ef548().Store(prevent_dce, 0u, 16u, MatrixLayout::RowMajor);
}

