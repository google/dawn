#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_left_i32_8x8 = Matrix<ComponentType::I32, 8, 8, MatrixUse::A, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
ByteAddressBuffer sb_ro : register(t1);
Matrix_left_i32_8x8 subgroupMatrixLoad_64f746() {
  uint arg_1 = 1u;
  uint arg_2 = 8u;
  uint v = arg_1;
  uint v_1 = max(arg_2, 8u);
  Matrix_left_i32_8x8 v_2 = Matrix_left_i32_8x8::Splat(int(0));
  if ((((v + (v_1 * 7u)) + 8u) <= 1024u)) {
    v_2 = Matrix_left_i32_8x8::Load(sb_ro, (0u + (v * 4u)), (v_1 * 4u), MatrixLayout::ColMajor);
  }
  Matrix_left_i32_8x8 res = v_2;
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixLoad_64f746().Store(prevent_dce, 0u, 256u, MatrixLayout::RowMajor);
}

