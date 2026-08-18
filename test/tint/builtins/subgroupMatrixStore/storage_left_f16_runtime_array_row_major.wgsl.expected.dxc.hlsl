#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_left_f16_8x8 = Matrix<ComponentType::F16, 8, 8, MatrixUse::A, MatrixScope::Wave>;

RWByteAddressBuffer out0 : register(u0);
RWByteAddressBuffer out1 : register(u1);
RWByteAddressBuffer out2 : register(u2);
RWByteAddressBuffer out3 : register(u3);
RWByteAddressBuffer out4 : register(u4);
RWByteAddressBuffer out5 : register(u5);
RWByteAddressBuffer out6 : register(u6);
[numthreads(64, 1, 1)]
void main() {
  Matrix_left_f16_8x8 m = Matrix_left_f16_8x8::Splat(float16_t(0.0h));
  uint v = 0u;
  out0.GetDimensions(v);
  bool v_1 = (((0u + (16u * 7u)) + 4u) <= (v / 4u));
  m.Store(out0, (0u + (select(v_1, 0u, 0u) * 4u)), (select(v_1, 16u, 4u) * 4u), MatrixLayout::RowMajor);
  uint v_2 = 0u;
  out1.GetDimensions(v_2);
  bool v_3 = (((0u + (16u * 7u)) + 2u) <= (v_2 / 8u));
  m.Store(out1, (0u + (select(v_3, 0u, 0u) * 8u)), (select(v_3, 16u, 2u) * 8u), MatrixLayout::RowMajor);
  uint v_4 = 0u;
  out2.GetDimensions(v_4);
  bool v_5 = (((0u + (16u * 7u)) + 1u) <= (v_4 / 16u));
  m.Store(out2, (0u + (select(v_5, 0u, 0u) * 16u)), (select(v_5, 16u, 1u) * 16u), MatrixLayout::RowMajor);
  uint v_6 = 0u;
  out3.GetDimensions(v_6);
  bool v_7 = (((0u + (16u * 7u)) + 1u) <= (v_6 / 16u));
  m.Store(out3, (0u + (select(v_7, 0u, 0u) * 16u)), (select(v_7, 16u, 1u) * 16u), MatrixLayout::RowMajor);
  uint v_8 = 0u;
  out4.GetDimensions(v_8);
  bool v_9 = (((0u + (16u * 7u)) + 8u) <= (v_8 / 2u));
  m.Store(out4, (0u + (select(v_9, 0u, 0u) * 2u)), (select(v_9, 16u, 8u) * 2u), MatrixLayout::RowMajor);
  uint v_10 = 0u;
  out5.GetDimensions(v_10);
  bool v_11 = (((0u + (16u * 7u)) + 4u) <= (v_10 / 4u));
  m.Store(out5, (0u + (select(v_11, 0u, 0u) * 4u)), (select(v_11, 16u, 4u) * 4u), MatrixLayout::RowMajor);
  uint v_12 = 0u;
  out6.GetDimensions(v_12);
  bool v_13 = (((0u + (16u * 7u)) + 2u) <= (v_12 / 8u));
  m.Store(out6, (0u + (select(v_13, 0u, 0u) * 8u)), (select(v_13, 16u, 2u) * 8u), MatrixLayout::RowMajor);
}

