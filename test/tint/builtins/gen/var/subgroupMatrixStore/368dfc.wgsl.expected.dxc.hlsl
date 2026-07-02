#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_result_f16_8x8 = Matrix<ComponentType::F16, 8, 8, MatrixUse::Accumulator, MatrixScope::Wave>;

RWByteAddressBuffer sb_rw : register(u0);
void subgroupMatrixStore_368dfc() {
  int arg_1 = int(1);
  Matrix_result_f16_8x8 arg_2 = Matrix_result_f16_8x8::Splat(float16_t(0.0h));
  uint arg_3 = 8u;
  int v = arg_1;
  Matrix_result_f16_8x8 v_1 = arg_2;
  uint v_2 = max(arg_3, 8u);
  if ((((asuint(v) + (v_2 * 7u)) + 8u) <= 1024u)) {
    v_1.Store(sb_rw, (0u + (uint(v) * 2u)), (v_2 * 2u), MatrixLayout::RowMajor);
  }
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixStore_368dfc();
}

