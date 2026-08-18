#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_result_u8_8x8 = Matrix<ComponentType::U8, 8, 8, MatrixUse::Accumulator, MatrixScope::Wave>;

RWByteAddressBuffer out0 : register(u0);
RWByteAddressBuffer out1 : register(u1);
RWByteAddressBuffer out2 : register(u2);
RWByteAddressBuffer out3 : register(u3);
RWByteAddressBuffer out4 : register(u4);
RWByteAddressBuffer out5 : register(u5);
RWByteAddressBuffer out6 : register(u6);
[numthreads(64, 1, 1)]
void main() {
  Matrix_result_u8_8x8 m = Matrix_result_u8_8x8::Splat(0u);
  m.Store(out0, 0u, 64u, MatrixLayout::RowMajor);
  m.Store(out1, 0u, 128u, MatrixLayout::RowMajor);
  m.Store(out2, 0u, 256u, MatrixLayout::RowMajor);
  m.Store(out3, 0u, 256u, MatrixLayout::RowMajor);
  m.Store(out4, 0u, 32u, MatrixLayout::RowMajor);
  m.Store(out5, 0u, 64u, MatrixLayout::RowMajor);
  m.Store(out6, 0u, 128u, MatrixLayout::RowMajor);
}

