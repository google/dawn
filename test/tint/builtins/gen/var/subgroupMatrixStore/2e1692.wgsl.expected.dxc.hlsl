#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_right_f32_8x8 = Matrix<ComponentType::F32, 8, 8, MatrixUse::B, MatrixScope::Wave>;

RWByteAddressBuffer sb_rw : register(u0);
void subgroupMatrixStore_2e1692() {
  uint arg_1 = 1u;
  Matrix_right_f32_8x8 arg_2 = Matrix_right_f32_8x8::Splat(0.0f);
  int arg_4 = int(8);
  uint v = arg_1;
  Matrix_right_f32_8x8 v_1 = arg_2;
  uint v_2 = max(asuint(arg_4), 8u);
  if ((((v + (v_2 * 7u)) + 8u) <= 1024u)) {
    v_1.Store(sb_rw, (0u + (v * 4u)), (v_2 * 4u), MatrixLayout::ColMajor);
  }
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixStore_2e1692();
}

