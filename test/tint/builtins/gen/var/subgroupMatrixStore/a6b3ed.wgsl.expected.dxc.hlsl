#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_right_u32_8x8 = Matrix<ComponentType::U32, 8, 8, MatrixUse::B, MatrixScope::Wave>;

RWByteAddressBuffer sb_rw : register(u0);
void subgroupMatrixStore_a6b3ed() {
  uint arg_1 = 1u;
  Matrix_right_u32_8x8 arg_2 = Matrix_right_u32_8x8::Splat(0u);
  uint arg_3 = 8u;
  uint v = arg_1;
  uint v_1 = max(arg_3, 8u);
  bool v_2 = (((v + (v_1 * 7u)) + 8u) <= 1024u);
  arg_2.Store(sb_rw, (0u + (select(v_2, v, 0u) * 4u)), (select(v_2, v_1, 8u) * 4u), MatrixLayout::ColMajor);
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixStore_a6b3ed();
}

