#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_result_i32_8x8 = Matrix<ComponentType::I32, 8, 8, MatrixUse::Accumulator, MatrixScope::Wave>;

RWByteAddressBuffer sb_rw : register(u0);
void subgroupMatrixStore_75fef7() {
  uint arg_1 = 1u;
  Matrix_result_i32_8x8 arg_2 = Matrix_result_i32_8x8::Splat(int(0));
  int arg_3 = int(8);
  uint v = arg_1;
  Matrix_result_i32_8x8 v_1 = arg_2;
  uint v_2 = max(asuint(arg_3), 8u);
  if ((((v + (v_2 * 7u)) + 8u) <= 1024u)) {
    v_1.Store(sb_rw, (0u + (v * 4u)), (v_2 * 4u), MatrixLayout::ColMajor);
  }
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixStore_75fef7();
}

