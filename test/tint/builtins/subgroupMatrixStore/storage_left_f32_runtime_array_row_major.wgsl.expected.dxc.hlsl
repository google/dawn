#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_left_f32_8x8 = Matrix<ComponentType::F32, 8, 8, MatrixUse::A, MatrixScope::Wave>;

RWByteAddressBuffer out0 : register(u0);
RWByteAddressBuffer out1 : register(u1);
RWByteAddressBuffer out2 : register(u2);
RWByteAddressBuffer out3 : register(u3);
RWByteAddressBuffer out5 : register(u5);
RWByteAddressBuffer out6 : register(u6);
[numthreads(64, 1, 1)]
void main() {
  Matrix_left_f32_8x8 m = Matrix_left_f32_8x8::Splat(0.0f);
  uint v = 0u;
  out0.GetDimensions(v);
  uint v_1 = asuint(int(0));
  uint v_2 = (v_1 + select((((16u != 0u) & (7u != 0u)) & (7u > (4294967295u / 16u))), 4294967295u, (16u * 7u)));
  uint v_3 = select((v_2 < v_1), 4294967295u, v_2);
  uint v_4 = (v_3 + 8u);
  bool v_5 = (select((v_4 < v_3), 4294967295u, v_4) <= (v / 4u));
  m.Store(out0, (0u + (select(v_5, v_1, 0u) * 4u)), (select(v_5, 16u, 8u) * 4u), MatrixLayout::RowMajor);
  uint v_6 = 0u;
  out1.GetDimensions(v_6);
  uint v_7 = asuint(int(0));
  uint v_8 = (v_7 + select((((16u != 0u) & (7u != 0u)) & (7u > (4294967295u / 16u))), 4294967295u, (16u * 7u)));
  uint v_9 = select((v_8 < v_7), 4294967295u, v_8);
  uint v_10 = (v_9 + 4u);
  bool v_11 = (select((v_10 < v_9), 4294967295u, v_10) <= (v_6 / 8u));
  m.Store(out1, (0u + (select(v_11, v_7, 0u) * 8u)), (select(v_11, 16u, 4u) * 8u), MatrixLayout::RowMajor);
  uint v_12 = 0u;
  out2.GetDimensions(v_12);
  uint v_13 = asuint(int(0));
  uint v_14 = (v_13 + select((((16u != 0u) & (7u != 0u)) & (7u > (4294967295u / 16u))), 4294967295u, (16u * 7u)));
  uint v_15 = select((v_14 < v_13), 4294967295u, v_14);
  uint v_16 = (v_15 + 2u);
  bool v_17 = (select((v_16 < v_15), 4294967295u, v_16) <= (v_12 / 16u));
  m.Store(out2, (0u + (select(v_17, v_13, 0u) * 16u)), (select(v_17, 16u, 2u) * 16u), MatrixLayout::RowMajor);
  uint v_18 = 0u;
  out3.GetDimensions(v_18);
  uint v_19 = asuint(int(0));
  uint v_20 = (v_19 + select((((16u != 0u) & (7u != 0u)) & (7u > (4294967295u / 16u))), 4294967295u, (16u * 7u)));
  uint v_21 = select((v_20 < v_19), 4294967295u, v_20);
  uint v_22 = (v_21 + 2u);
  bool v_23 = (select((v_22 < v_21), 4294967295u, v_22) <= (v_18 / 16u));
  m.Store(out3, (0u + (select(v_23, v_19, 0u) * 16u)), (select(v_23, 16u, 2u) * 16u), MatrixLayout::RowMajor);
  uint v_24 = 0u;
  out5.GetDimensions(v_24);
  uint v_25 = asuint(int(0));
  uint v_26 = (v_25 + select((((16u != 0u) & (7u != 0u)) & (7u > (4294967295u / 16u))), 4294967295u, (16u * 7u)));
  uint v_27 = select((v_26 < v_25), 4294967295u, v_26);
  uint v_28 = (v_27 + 8u);
  bool v_29 = (select((v_28 < v_27), 4294967295u, v_28) <= (v_24 / 4u));
  m.Store(out5, (0u + (select(v_29, v_25, 0u) * 4u)), (select(v_29, 16u, 8u) * 4u), MatrixLayout::RowMajor);
  uint v_30 = 0u;
  out6.GetDimensions(v_30);
  uint v_31 = asuint(int(0));
  uint v_32 = (v_31 + select((((16u != 0u) & (7u != 0u)) & (7u > (4294967295u / 16u))), 4294967295u, (16u * 7u)));
  uint v_33 = select((v_32 < v_31), 4294967295u, v_32);
  uint v_34 = (v_33 + 4u);
  bool v_35 = (select((v_34 < v_33), 4294967295u, v_34) <= (v_30 / 8u));
  m.Store(out6, (0u + (select(v_35, v_31, 0u) * 8u)), (select(v_35, 16u, 4u) * 8u), MatrixLayout::RowMajor);
}

