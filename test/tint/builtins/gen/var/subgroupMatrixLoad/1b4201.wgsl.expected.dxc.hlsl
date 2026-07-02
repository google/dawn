#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_result_i32_8x8 = Matrix<ComponentType::I32, 8, 8, MatrixUse::Accumulator, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
RWByteAddressBuffer sb_rw : register(u1);
Matrix_result_i32_8x8 subgroupMatrixLoad_1b4201() {
  uint arg_1 = 1u;
  int arg_2 = int(8);
  uint v = arg_1;
  uint v_1 = max(asuint(arg_2), 8u);
  Matrix_result_i32_8x8 v_2 = Matrix_result_i32_8x8::Splat(int(0));
  if ((((v + (v_1 * 7u)) + 8u) <= 1024u)) {
    v_2 = Matrix_result_i32_8x8::Load(sb_rw, (0u + (v * 4u)), (v_1 * 4u), MatrixLayout::ColMajor);
  }
  Matrix_result_i32_8x8 res = v_2;
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixLoad_1b4201().Store(prevent_dce, 0u, 32u, MatrixLayout::RowMajor);
}

