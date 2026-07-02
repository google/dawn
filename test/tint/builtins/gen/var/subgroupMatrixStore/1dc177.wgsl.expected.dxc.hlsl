#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_left_u8_8x8 = Matrix<ComponentType::U8, 8, 8, MatrixUse::A, MatrixScope::Wave>;

RWByteAddressBuffer sb_rw : register(u0);
void subgroupMatrixStore_1dc177() {
  int arg_1 = int(1);
  Matrix_left_u8_8x8 arg_2 = Matrix_left_u8_8x8::Splat(0u);
  int arg_4 = int(8);
  int v = arg_1;
  Matrix_left_u8_8x8 v_1 = arg_2;
  uint v_2 = max(asuint(arg_4), 8u);
  if ((((asuint(v) + (v_2 * 7u)) + 8u) <= 4096u)) {
    v_1.Store(sb_rw, (0u + (uint(v) * 1u)), (v_2 * 1u), MatrixLayout::ColMajor);
  }
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixStore_1dc177();
}

