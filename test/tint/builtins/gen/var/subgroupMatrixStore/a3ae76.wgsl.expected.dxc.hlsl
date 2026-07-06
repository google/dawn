#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_result_f16_8x8 = Matrix<ComponentType::F16, 8, 8, MatrixUse::Accumulator, MatrixScope::Wave>;

RWByteAddressBuffer sb_rw : register(u0);
void subgroupMatrixStore_a3ae76() {
  uint arg_1 = 1u;
  Matrix_result_f16_8x8 arg_2 = Matrix_result_f16_8x8::Splat(float16_t(0.0h));
  int arg_3 = int(8);
  uint v = arg_1;
  uint v_1 = max(asuint(arg_3), 8u);
  bool v_2 = (((v + (v_1 * 7u)) + 8u) <= 1024u);
  arg_2.Store(sb_rw, (0u + (select(v_2, v, 0u) * 2u)), (select(v_2, v_1, 8u) * 2u), MatrixLayout::RowMajor);
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixStore_a3ae76();
}

