#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_right_i8_8x8 = Matrix<ComponentType::I8, 8, 8, MatrixUse::B, MatrixScope::Wave>;

RWByteAddressBuffer out0 : register(u0);
RWByteAddressBuffer out1 : register(u1);
RWByteAddressBuffer out2 : register(u2);
RWByteAddressBuffer out3 : register(u3);
RWByteAddressBuffer out4 : register(u4);
RWByteAddressBuffer out5 : register(u5);
RWByteAddressBuffer out6 : register(u6);
[numthreads(64, 1, 1)]
void main() {
  Matrix_right_i8_8x8 m = Matrix_right_i8_8x8::Splat(int(0));
  uint v = 0u;
  out0.GetDimensions(v);
  uint v_1 = asuint(int(0));
  bool v_2 = (((v_1 + (16u * 7u)) + 2u) <= (v / 4u));
  m.Store(out0, (0u + (select(v_2, v_1, 0u) * 4u)), (select(v_2, 16u, 2u) * 4u), MatrixLayout::ColMajor);
  uint v_3 = 0u;
  out1.GetDimensions(v_3);
  uint v_4 = asuint(int(0));
  bool v_5 = (((v_4 + (16u * 7u)) + 1u) <= (v_3 / 8u));
  m.Store(out1, (0u + (select(v_5, v_4, 0u) * 4u)), (select(v_5, 16u, 1u) * 4u), MatrixLayout::ColMajor);
  uint v_6 = 0u;
  out2.GetDimensions(v_6);
  uint v_7 = asuint(int(0));
  bool v_8 = (((v_7 + (16u * 7u)) + 1u) <= (v_6 / 16u));
  m.Store(out2, (0u + (select(v_8, v_7, 0u) * 4u)), (select(v_8, 16u, 1u) * 4u), MatrixLayout::ColMajor);
  uint v_9 = 0u;
  out3.GetDimensions(v_9);
  uint v_10 = asuint(int(0));
  bool v_11 = (((v_10 + (16u * 7u)) + 1u) <= (v_9 / 16u));
  m.Store(out3, (0u + (select(v_11, v_10, 0u) * 4u)), (select(v_11, 16u, 1u) * 4u), MatrixLayout::ColMajor);
  uint v_12 = 0u;
  out4.GetDimensions(v_12);
  uint v_13 = asuint(int(0));
  bool v_14 = (((v_13 + (16u * 7u)) + 4u) <= (v_12 / 2u));
  m.Store(out4, (0u + (select(v_14, v_13, 0u) * 4u)), (select(v_14, 16u, 4u) * 4u), MatrixLayout::ColMajor);
  uint v_15 = 0u;
  out5.GetDimensions(v_15);
  uint v_16 = asuint(int(0));
  bool v_17 = (((v_16 + (16u * 7u)) + 2u) <= (v_15 / 4u));
  m.Store(out5, (0u + (select(v_17, v_16, 0u) * 4u)), (select(v_17, 16u, 2u) * 4u), MatrixLayout::ColMajor);
  uint v_18 = 0u;
  out6.GetDimensions(v_18);
  uint v_19 = asuint(int(0));
  bool v_20 = (((v_19 + (16u * 7u)) + 1u) <= (v_18 / 8u));
  m.Store(out6, (0u + (select(v_20, v_19, 0u) * 4u)), (select(v_20, 16u, 1u) * 4u), MatrixLayout::ColMajor);
}

