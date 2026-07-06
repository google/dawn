#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_left_u32_8x8 = Matrix<ComponentType::U32, 8, 8, MatrixUse::A, MatrixScope::Wave>;

RWByteAddressBuffer sb_rw : register(u0);
void subgroupMatrixStore_f21bc5() {
  int arg_1 = int(1);
  Matrix_left_u32_8x8 arg_2 = Matrix_left_u32_8x8::Splat(0u);
  int arg_3 = int(8);
  uint v = max(asuint(arg_3), 8u);
  uint v_1 = asuint(arg_1);
  bool v_2 = (((v_1 + (v * 7u)) + 8u) <= 1024u);
  arg_2.Store(sb_rw, (0u + (select(v_2, v_1, 0u) * 4u)), (select(v_2, v, 8u) * 4u), MatrixLayout::RowMajor);
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixStore_f21bc5();
}

