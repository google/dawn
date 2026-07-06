#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_right_f16_8x8 = Matrix<ComponentType::F16, 8, 8, MatrixUse::B, MatrixScope::Wave>;

RWByteAddressBuffer sb_rw : register(u0);
void subgroupMatrixStore_455791() {
  int arg_1 = int(1);
  Matrix_right_f16_8x8 arg_2 = Matrix_right_f16_8x8::Splat(float16_t(0.0h));
  uint arg_3 = 8u;
  uint v = max(arg_3, 8u);
  uint v_1 = asuint(arg_1);
  bool v_2 = (((v_1 + (v * 7u)) + 8u) <= 1024u);
  arg_2.Store(sb_rw, (0u + (select(v_2, v_1, 0u) * 2u)), (select(v_2, v, 8u) * 2u), MatrixLayout::RowMajor);
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixStore_455791();
}

