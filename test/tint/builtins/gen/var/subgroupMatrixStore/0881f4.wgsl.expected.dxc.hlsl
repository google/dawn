#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_left_i8_8x8 = Matrix<ComponentType::I8, 8, 8, MatrixUse::A, MatrixScope::Wave>;

RWByteAddressBuffer sb_rw : register(u0);
void subgroupMatrixStore_0881f4() {
  uint arg_1 = 1u;
  Matrix_left_i8_8x8 arg_2 = Matrix_left_i8_8x8::Splat(int(0));
  uint arg_3 = 8u;
  uint v = arg_1;
  Matrix_left_i8_8x8 v_1 = arg_2;
  uint v_2 = max(arg_3, 8u);
  if ((((v + (v_2 * 7u)) + 8u) <= 4096u)) {
    v_1.Store(sb_rw, (0u + (v * 1u)), (v_2 * 1u), MatrixLayout::ColMajor);
  }
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupMatrixStore_0881f4();
}

