#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_left_u8_8x8 = Matrix<ComponentType::U8, 8, 8, MatrixUse::A, MatrixScope::Wave>;

RWByteAddressBuffer sb_rw : register(u0);
void subgroupMatrixStore_bef52e() {
  uint arg_1 = 1u;
  Matrix_left_u8_8x8 arg_2 = Matrix_left_u8_8x8::Splat(0u);
  uint arg_3 = 8u;
  uint v = arg_1;
  Matrix_left_u8_8x8 v_1 = arg_2;
  uint v_2 = max(arg_3, 8u);
  if ((((v + (v_2 * 7u)) + 8u) <= 4096u)) {
    v_1.Store(sb_rw, (0u + (v * 1u)), (v_2 * 1u), MatrixLayout::RowMajor);
  }
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixStore_bef52e();
}

