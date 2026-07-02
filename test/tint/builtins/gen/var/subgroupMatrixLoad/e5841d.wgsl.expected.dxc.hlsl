#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_result_f32_8x8 = Matrix<ComponentType::F32, 8, 8, MatrixUse::Accumulator, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
RWByteAddressBuffer sb_rw : register(u1);
Matrix_result_f32_8x8 subgroupMatrixLoad_e5841d() {
  int arg_1 = int(1);
  uint arg_2 = 8u;
  int v = arg_1;
  uint v_1 = max(arg_2, 8u);
  Matrix_result_f32_8x8 v_2 = Matrix_result_f32_8x8::Splat(0.0f);
  if ((((asuint(v) + (v_1 * 7u)) + 8u) <= 1024u)) {
    v_2 = Matrix_result_f32_8x8::Load(sb_rw, (0u + (uint(v) * 4u)), (v_1 * 4u), MatrixLayout::RowMajor);
  }
  Matrix_result_f32_8x8 res = v_2;
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixLoad_e5841d().Store(prevent_dce, 0u, 32u, MatrixLayout::RowMajor);
}

