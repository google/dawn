#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_result_u32_8x8 = Matrix<ComponentType::U32, 8, 8, MatrixUse::Accumulator, MatrixScope::Wave>;

RWByteAddressBuffer sb_rw : register(u0);
void subgroupMatrixStore_be9b0a() {
  int arg_1 = int(1);
  Matrix_result_u32_8x8 arg_2 = Matrix_result_u32_8x8::Splat(0u);
  int arg_3 = int(8);
  int v = arg_1;
  Matrix_result_u32_8x8 v_1 = arg_2;
  uint v_2 = max(asuint(arg_3), 8u);
  if ((((asuint(v) + (v_2 * 7u)) + 8u) <= 1024u)) {
    v_1.Store(sb_rw, (0u + (uint(v) * 4u)), (v_2 * 4u), MatrixLayout::RowMajor);
  }
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixStore_be9b0a();
}

