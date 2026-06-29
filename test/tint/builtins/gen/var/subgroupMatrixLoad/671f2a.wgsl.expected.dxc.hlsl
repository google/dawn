#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_left_u8_8x8 = Matrix<ComponentType::U8, 8, 8, MatrixUse::A, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
RWByteAddressBuffer sb_rw : register(u1);
Matrix_left_u8_8x8 subgroupMatrixLoad_671f2a() {
  uint arg_1 = 1u;
  uint arg_2 = 8u;
  uint v = arg_1;
  uint v_1 = max(arg_2, 8u);
  Matrix_left_u8_8x8 v_2 = Matrix_left_u8_8x8::Splat(0u);
  if ((((v + (v_1 * 7u)) + 8u) <= 4096u)) {
    v_2 = Matrix_left_u8_8x8::Load(sb_rw, (0u + (v * 1u)), (v_1 * 1u), MatrixLayout::RowMajor);
  }
  Matrix_left_u8_8x8 res = v_2;
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixLoad_671f2a().Store(prevent_dce, 0u, 8u, MatrixLayout::RowMajor);
}

