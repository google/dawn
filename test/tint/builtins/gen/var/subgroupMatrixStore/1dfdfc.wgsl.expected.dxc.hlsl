#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_left_u32_8x8 = Matrix<ComponentType::U32, 8, 8, MatrixUse::A, MatrixScope::Wave>;

RWByteAddressBuffer sb_rw : register(u0);
void subgroupMatrixStore_1dfdfc() {
  int arg_1 = int(1);
  Matrix_left_u32_8x8 arg_2 = Matrix_left_u32_8x8::Splat(0u);
  uint arg_3 = 8u;
  int v = arg_1;
  Matrix_left_u32_8x8 v_1 = arg_2;
  uint v_2 = max(arg_3, 8u);
  if ((((asuint(v) + (v_2 * 7u)) + 8u) <= 1024u)) {
    v_1.Store(sb_rw, (0u + (uint(v) * 4u)), (v_2 * 4u), MatrixLayout::ColMajor);
  }
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixStore_1dfdfc();
}

