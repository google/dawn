#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_right_i8_8x8 = Matrix<ComponentType::I8, 8, 8, MatrixUse::B, MatrixScope::Wave>;

RWByteAddressBuffer sb_rw : register(u0);
void subgroupMatrixStore_170c15() {
  int arg_1 = int(1);
  Matrix_right_i8_8x8 arg_2 = Matrix_right_i8_8x8::Splat(int(0));
  uint arg_3 = 8u;
  uint v = max(arg_3, 2u);
  uint v_1 = asuint(arg_1);
  bool v_2 = (((v_1 + (v * 7u)) + 2u) <= 1024u);
  arg_2.Store(sb_rw, (0u + (select(v_2, v_1, 0u) * 4u)), (select(v_2, v, 2u) * 4u), MatrixLayout::ColMajor);
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixStore_170c15();
}

