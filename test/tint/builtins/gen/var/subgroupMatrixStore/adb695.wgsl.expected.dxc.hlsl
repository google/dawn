#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_right_i8_8x8 = Matrix<ComponentType::I8, 8, 8, MatrixUse::B, MatrixScope::Wave>;

RWByteAddressBuffer sb_rw : register(u0);
void subgroupMatrixStore_adb695() {
  uint arg_1 = 1u;
  Matrix_right_i8_8x8 arg_2 = Matrix_right_i8_8x8::Splat(int(0));
  uint arg_3 = 8u;
  uint v = arg_1;
  Matrix_right_i8_8x8 v_1 = arg_2;
  uint v_2 = max(arg_3, 2u);
  if ((((v + (v_2 * 7u)) + 2u) <= 1024u)) {
    v_1.Store(sb_rw, (0u + (v * 4u)), (v_2 * 4u), MatrixLayout::RowMajor);
  }
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixStore_adb695();
}

