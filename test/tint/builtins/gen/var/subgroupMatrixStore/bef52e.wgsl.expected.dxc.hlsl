#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_left_u8_8x8 = Matrix<ComponentType::U8, 8, 8, MatrixUse::A, MatrixScope::Wave>;

RWByteAddressBuffer sb_rw : register(u0);
void subgroupMatrixStore_bef52e() {
  uint arg_1 = 1u;
  Matrix_left_u8_8x8 arg_2 = Matrix_left_u8_8x8::Splat(0u);
  uint arg_3 = 8u;
  uint v = arg_1;
  uint v_1 = max(arg_3, 2u);
  bool v_2 = (((v + (v_1 * 7u)) + 2u) <= 1024u);
  arg_2.Store(sb_rw, (0u + (select(v_2, v, 0u) * 4u)), (select(v_2, v_1, 2u) * 4u), MatrixLayout::RowMajor);
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixStore_bef52e();
}

