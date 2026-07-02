#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_right_i8_8x8 = Matrix<ComponentType::I8, 8, 8, MatrixUse::B, MatrixScope::Wave>;

RWByteAddressBuffer prevent_dce : register(u0);
ByteAddressBuffer sb_ro : register(t1);
Matrix_right_i8_8x8 subgroupMatrixLoad_6faedf() {
  int arg_1 = int(1);
  int arg_2 = int(8);
  int v = arg_1;
  uint v_1 = max(asuint(arg_2), 2u);
  Matrix_right_i8_8x8 v_2 = Matrix_right_i8_8x8::Splat(int(0));
  if ((((asuint(v) + (v_1 * 7u)) + 2u) <= 1024u)) {
    v_2 = Matrix_right_i8_8x8::Load(sb_ro, (0u + (uint(v) * 4u)), (v_1 * 4u), MatrixLayout::RowMajor);
  }
  Matrix_right_i8_8x8 res = v_2;
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixLoad_6faedf().Store(prevent_dce, 0u, 8u, MatrixLayout::RowMajor);
}

