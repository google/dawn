#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_right_i8_8x8 = Matrix<ComponentType::I8, 8, 8, MatrixUse::B, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
RWByteAddressBuffer sb_rw : register(u1);
Matrix_right_i8_8x8 subgroupMatrixLoad_700c62() {
  int arg_1 = int(1);
  uint arg_3 = 8u;
  int v = arg_1;
  uint v_1 = max(arg_3, 8u);
  Matrix_right_i8_8x8 v_2 = Matrix_right_i8_8x8::Splat(int(0));
  if ((((asuint(v) + (v_1 * 7u)) + 8u) <= 4096u)) {
    v_2 = Matrix_right_i8_8x8::Load(sb_rw, (0u + (uint(v) * 1u)), (v_1 * 1u), MatrixLayout::ColMajor);
  }
  Matrix_right_i8_8x8 res = v_2;
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixLoad_700c62().Store(prevent_dce, 0u, 8u, MatrixLayout::RowMajor);
}

