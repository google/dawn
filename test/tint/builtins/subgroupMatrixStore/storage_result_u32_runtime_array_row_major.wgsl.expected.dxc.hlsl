#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_result_u32_8x8 = Matrix<ComponentType::U32, 8, 8, MatrixUse::Accumulator, MatrixScope::Wave>;

RWByteAddressBuffer out0 : register(u0);
RWByteAddressBuffer out1 : register(u1);
RWByteAddressBuffer out2 : register(u2);
RWByteAddressBuffer out3 : register(u3);
RWByteAddressBuffer out5 : register(u5);
RWByteAddressBuffer out6 : register(u6);
[numthreads(64, 1, 1)]
void main() {
  Matrix_result_u32_8x8 m = Matrix_result_u32_8x8::Splat(0u);
  uint v = 0u;
  out0.GetDimensions(v);
  uint v_1 = asuint(int(0));
  bool v_2 = (((v_1 + (16u * 7u)) + 8u) <= (v / 4u));
  m.Store(out0, (0u + (select(v_2, v_1, 0u) * 4u)), (select(v_2, 16u, 8u) * 4u), MatrixLayout::RowMajor);
  uint v_3 = 0u;
  out1.GetDimensions(v_3);
  uint v_4 = asuint(int(0));
  bool v_5 = ((((v_4 + (16u * 7u)) * 2u) + 8u) <= ((v_3 / 8u) * 2u));
  m.Store(out1, (0u + (select(v_5, v_4, 0u) * 4u)), (select(v_5, 16u, 4u) * 4u), MatrixLayout::RowMajor);
  uint v_6 = 0u;
  out2.GetDimensions(v_6);
  uint v_7 = asuint(int(0));
  bool v_8 = ((((v_7 + (16u * 7u)) * 4u) + 8u) <= ((v_6 / 16u) * 4u));
  m.Store(out2, (0u + (select(v_8, v_7, 0u) * 4u)), (select(v_8, 16u, 2u) * 4u), MatrixLayout::RowMajor);
  uint v_9 = 0u;
  out3.GetDimensions(v_9);
  uint v_10 = asuint(int(0));
  bool v_11 = ((((v_10 + (16u * 7u)) * 4u) + 8u) <= ((v_9 / 16u) * 4u));
  m.Store(out3, (0u + (select(v_11, v_10, 0u) * 4u)), (select(v_11, 16u, 2u) * 4u), MatrixLayout::RowMajor);
  uint v_12 = 0u;
  out5.GetDimensions(v_12);
  uint v_13 = asuint(int(0));
  bool v_14 = (((v_13 + (16u * 7u)) + 8u) <= (v_12 / 4u));
  m.Store(out5, (0u + (select(v_14, v_13, 0u) * 4u)), (select(v_14, 16u, 8u) * 4u), MatrixLayout::RowMajor);
  uint v_15 = 0u;
  out6.GetDimensions(v_15);
  uint v_16 = asuint(int(0));
  bool v_17 = ((((v_16 + (16u * 7u)) * 2u) + 8u) <= ((v_15 / 8u) * 2u));
  m.Store(out6, (0u + (select(v_17, v_16, 0u) * 4u)), (select(v_17, 16u, 4u) * 4u), MatrixLayout::RowMajor);
}

