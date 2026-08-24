#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_left_i8_8x8 = Matrix<ComponentType::I8, 8, 8, MatrixUse::A, MatrixScope::Wave>;
using Matrix_right_u8_8x8 = Matrix<ComponentType::U8, 8, 8, MatrixUse::B, MatrixScope::Wave>;

RWByteAddressBuffer ibuffer : register(u0);
RWByteAddressBuffer ubuffer : register(u1);
[numthreads(64, 1, 1)]
void main() {
  uint v = 0u;
  ibuffer.GetDimensions(v);
  uint v_1 = asuint(int(0));
  uint v_2 = asuint(int(64));
  uint v_3 = (v_1 + select((((v_2 != 0u) & (7u != 0u)) & (7u > (4294967295u / v_2))), 4294967295u, (v_2 * 7u)));
  uint v_4 = select((v_3 < v_1), 4294967295u, v_3);
  uint v_5 = (v_4 + 2u);
  bool v_6 = (select((v_5 < v_4), 4294967295u, v_5) <= (v / 4u));
  Matrix_left_i8_8x8::Splat(int(0)).Store(ibuffer, (0u + (select(v_6, v_1, 0u) * 4u)), (select(v_6, v_2, 2u) * 4u), MatrixLayout::RowMajor);
  uint v_7 = 0u;
  ubuffer.GetDimensions(v_7);
  uint v_8 = asuint(int(0));
  uint v_9 = asuint(int(64));
  uint v_10 = (v_8 + select((((v_9 != 0u) & (7u != 0u)) & (7u > (4294967295u / v_9))), 4294967295u, (v_9 * 7u)));
  uint v_11 = select((v_10 < v_8), 4294967295u, v_10);
  uint v_12 = (v_11 + 2u);
  bool v_13 = (select((v_12 < v_11), 4294967295u, v_12) <= (v_7 / 4u));
  Matrix_right_u8_8x8::Splat(0u).Store(ubuffer, (0u + (select(v_13, v_8, 0u) * 4u)), (select(v_13, v_9, 2u) * 4u), MatrixLayout::RowMajor);
  uint v_14 = 0u;
  ibuffer.GetDimensions(v_14);
  uint v_15 = asuint(int(0));
  uint v_16 = asuint(int(64));
  uint v_17 = (v_15 + select((((v_16 != 0u) & (7u != 0u)) & (7u > (4294967295u / v_16))), 4294967295u, (v_16 * 7u)));
  uint v_18 = select((v_17 < v_15), 4294967295u, v_17);
  uint v_19 = (v_18 + 2u);
  bool v_20 = (select((v_19 < v_18), 4294967295u, v_19) <= (v_14 / 4u));
  Matrix_left_i8_8x8::Splat(int(-42)).Store(ibuffer, (0u + (select(v_20, v_15, 0u) * 4u)), (select(v_20, v_16, 2u) * 4u), MatrixLayout::RowMajor);
  uint v_21 = 0u;
  ubuffer.GetDimensions(v_21);
  uint v_22 = asuint(int(0));
  uint v_23 = asuint(int(64));
  uint v_24 = (v_22 + select((((v_23 != 0u) & (7u != 0u)) & (7u > (4294967295u / v_23))), 4294967295u, (v_23 * 7u)));
  uint v_25 = select((v_24 < v_22), 4294967295u, v_24);
  uint v_26 = (v_25 + 2u);
  bool v_27 = (select((v_26 < v_25), 4294967295u, v_26) <= (v_21 / 4u));
  Matrix_right_u8_8x8::Splat(42u).Store(ubuffer, (0u + (select(v_27, v_22, 0u) * 4u)), (select(v_27, v_23, 2u) * 4u), MatrixLayout::RowMajor);
}

