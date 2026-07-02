#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_result_i8_8x8 = Matrix<ComponentType::I8, 8, 8, MatrixUse::Accumulator, MatrixScope::Wave>;

RWByteAddressBuffer sb_rw : register(u0);
void subgroupMatrixStore_52b5fe() {
  int arg_1 = int(1);
  Matrix_result_i8_8x8 arg_2 = Matrix_result_i8_8x8::Splat(int(0));
  uint arg_4 = 8u;
  int v = arg_1;
  Matrix_result_i8_8x8 v_1 = arg_2;
  uint v_2 = max(arg_4, 8u);
  if ((((asuint(v) + (v_2 * 7u)) + 8u) <= 4096u)) {
    v_1.Store(sb_rw, (0u + (uint(v) * 1u)), (v_2 * 1u), MatrixLayout::ColMajor);
  }
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixStore_52b5fe();
}

