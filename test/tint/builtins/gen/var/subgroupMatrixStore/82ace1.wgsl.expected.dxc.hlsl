#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_result_f16_8x8 = Matrix<ComponentType::F16, 8, 8, MatrixUse::Accumulator, MatrixScope::Wave>;

RWByteAddressBuffer sb_rw : register(u0);
void subgroupMatrixStore_82ace1() {
  uint arg_1 = 1u;
  Matrix_result_f16_8x8 arg_2 = Matrix_result_f16_8x8::Splat(float16_t(0.0h));
  uint arg_3 = 8u;
  uint v = arg_1;
  uint v_1 = max(arg_3, 8u);
  bool v_2 = (((v + (v_1 * 7u)) + 8u) <= 1024u);
  arg_2.Store(sb_rw, (0u + (select(v_2, v, 0u) * 2u)), (select(v_2, v_1, 8u) * 2u), MatrixLayout::RowMajor);
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixStore_82ace1();
}

