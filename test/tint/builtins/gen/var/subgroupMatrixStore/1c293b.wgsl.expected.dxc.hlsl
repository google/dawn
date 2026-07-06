#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_left_f16_8x8 = Matrix<ComponentType::F16, 8, 8, MatrixUse::A, MatrixScope::Wave>;

RWByteAddressBuffer sb_rw : register(u0);
void subgroupMatrixStore_1c293b() {
  int arg_1 = int(1);
  Matrix_left_f16_8x8 arg_2 = Matrix_left_f16_8x8::Splat(float16_t(0.0h));
  int arg_3 = int(8);
  uint v = max(asuint(arg_3), 8u);
  uint v_1 = asuint(arg_1);
  bool v_2 = (((v_1 + (v * 7u)) + 8u) <= 1024u);
  arg_2.Store(sb_rw, (0u + (select(v_2, v_1, 0u) * 2u)), (select(v_2, v, 8u) * 2u), MatrixLayout::ColMajor);
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixStore_1c293b();
}

