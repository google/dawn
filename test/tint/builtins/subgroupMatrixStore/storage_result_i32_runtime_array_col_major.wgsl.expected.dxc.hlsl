#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_result_i32_8x8 = Matrix<ComponentType::I32, 8, 8, MatrixUse::Accumulator, MatrixScope::Wave>;

RWByteAddressBuffer out0 : register(u0);
RWByteAddressBuffer out1 : register(u1);
RWByteAddressBuffer out2 : register(u2);
RWByteAddressBuffer out3 : register(u3);
RWByteAddressBuffer out5 : register(u5);
RWByteAddressBuffer out6 : register(u6);
[numthreads(64, 1, 1)]
void main() {
  Matrix_result_i32_8x8 m = Matrix_result_i32_8x8::Splat(int(0));
  uint v = 0u;
  out0.GetDimensions(v);
  uint v_1 = (0u + select((((16u != 0u) & (7u != 0u)) & (7u > (4294967295u / 16u))), 4294967295u, (16u * 7u)));
  uint v_2 = select((v_1 < 0u), 4294967295u, v_1);
  uint v_3 = (v_2 + 8u);
  bool v_4 = (select((v_3 < v_2), 4294967295u, v_3) <= (v / 4u));
  m.Store(out0, (0u + (select(v_4, 0u, 0u) * 4u)), (select(v_4, 16u, 8u) * 4u), MatrixLayout::ColMajor);
  uint v_5 = 0u;
  out1.GetDimensions(v_5);
  uint v_6 = (0u + select((((16u != 0u) & (7u != 0u)) & (7u > (4294967295u / 16u))), 4294967295u, (16u * 7u)));
  uint v_7 = select((v_6 < 0u), 4294967295u, v_6);
  uint v_8 = (v_7 + 4u);
  bool v_9 = (select((v_8 < v_7), 4294967295u, v_8) <= (v_5 / 8u));
  m.Store(out1, (0u + (select(v_9, 0u, 0u) * 8u)), (select(v_9, 16u, 4u) * 8u), MatrixLayout::ColMajor);
  uint v_10 = 0u;
  out2.GetDimensions(v_10);
  uint v_11 = (0u + select((((16u != 0u) & (7u != 0u)) & (7u > (4294967295u / 16u))), 4294967295u, (16u * 7u)));
  uint v_12 = select((v_11 < 0u), 4294967295u, v_11);
  uint v_13 = (v_12 + 2u);
  bool v_14 = (select((v_13 < v_12), 4294967295u, v_13) <= (v_10 / 16u));
  m.Store(out2, (0u + (select(v_14, 0u, 0u) * 16u)), (select(v_14, 16u, 2u) * 16u), MatrixLayout::ColMajor);
  uint v_15 = 0u;
  out3.GetDimensions(v_15);
  uint v_16 = (0u + select((((16u != 0u) & (7u != 0u)) & (7u > (4294967295u / 16u))), 4294967295u, (16u * 7u)));
  uint v_17 = select((v_16 < 0u), 4294967295u, v_16);
  uint v_18 = (v_17 + 2u);
  bool v_19 = (select((v_18 < v_17), 4294967295u, v_18) <= (v_15 / 16u));
  m.Store(out3, (0u + (select(v_19, 0u, 0u) * 16u)), (select(v_19, 16u, 2u) * 16u), MatrixLayout::ColMajor);
  uint v_20 = 0u;
  out5.GetDimensions(v_20);
  uint v_21 = (0u + select((((16u != 0u) & (7u != 0u)) & (7u > (4294967295u / 16u))), 4294967295u, (16u * 7u)));
  uint v_22 = select((v_21 < 0u), 4294967295u, v_21);
  uint v_23 = (v_22 + 8u);
  bool v_24 = (select((v_23 < v_22), 4294967295u, v_23) <= (v_20 / 4u));
  m.Store(out5, (0u + (select(v_24, 0u, 0u) * 4u)), (select(v_24, 16u, 8u) * 4u), MatrixLayout::ColMajor);
  uint v_25 = 0u;
  out6.GetDimensions(v_25);
  uint v_26 = (0u + select((((16u != 0u) & (7u != 0u)) & (7u > (4294967295u / 16u))), 4294967295u, (16u * 7u)));
  uint v_27 = select((v_26 < 0u), 4294967295u, v_26);
  uint v_28 = (v_27 + 4u);
  bool v_29 = (select((v_28 < v_27), 4294967295u, v_28) <= (v_25 / 8u));
  m.Store(out6, (0u + (select(v_29, 0u, 0u) * 8u)), (select(v_29, 16u, 4u) * 8u), MatrixLayout::ColMajor);
}

