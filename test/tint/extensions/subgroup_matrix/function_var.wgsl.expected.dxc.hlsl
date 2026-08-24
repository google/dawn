#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_left_f32_8x8 = Matrix<ComponentType::F32, 8, 8, MatrixUse::A, MatrixScope::Wave>;
using Matrix_right_f32_8x8 = Matrix<ComponentType::F32, 8, 8, MatrixUse::B, MatrixScope::Wave>;
struct S {
  Matrix_left_f32_8x8 l;
  Matrix_right_f32_8x8 r;
};

struct S_Nested {
  S s;
};


RWByteAddressBuffer buffer : register(u0);
[numthreads(64, 1, 1)]
void main() {
  Matrix_left_f32_8x8 m = Matrix_left_f32_8x8::Splat(0.0f);
  Matrix_left_f32_8x8 v = Matrix_left_f32_8x8::Splat(0.0f);
  Matrix_left_f32_8x8 m_array[4] = {v, v, v, v};
  Matrix_left_f32_8x8 v_1 = Matrix_left_f32_8x8::Splat(0.0f);
  Matrix_left_f32_8x8 v_2[4] = {v_1, v_1, v_1, v_1};
  Matrix_left_f32_8x8 m_nested_array[4][4] = {v_2, v_2, v_2, v_2};
  S m_struct = {Matrix_left_f32_8x8::Splat(0.0f), Matrix_right_f32_8x8::Splat(0.0f)};
  S v_3 = {Matrix_left_f32_8x8::Splat(0.0f), Matrix_right_f32_8x8::Splat(0.0f)};
  S_Nested m_nested_struct = {v_3};
  uint v_4 = 0u;
  buffer.GetDimensions(v_4);
  uint v_5 = asuint(int(0));
  uint v_6 = asuint(int(64));
  uint v_7 = (v_5 + select((((v_6 != 0u) & (7u != 0u)) & (7u > (4294967295u / v_6))), 4294967295u, (v_6 * 7u)));
  uint v_8 = select((v_7 < v_5), 4294967295u, v_7);
  uint v_9 = (v_8 + 8u);
  bool v_10 = (select((v_9 < v_8), 4294967295u, v_9) <= (v_4 / 4u));
  m.Store(buffer, (0u + (select(v_10, v_5, 0u) * 4u)), (select(v_10, v_6, 8u) * 4u), MatrixLayout::RowMajor);
  uint v_11 = 0u;
  buffer.GetDimensions(v_11);
  uint v_12 = asuint(int(0));
  uint v_13 = asuint(int(64));
  uint v_14 = (v_12 + select((((v_13 != 0u) & (7u != 0u)) & (7u > (4294967295u / v_13))), 4294967295u, (v_13 * 7u)));
  uint v_15 = select((v_14 < v_12), 4294967295u, v_14);
  uint v_16 = (v_15 + 8u);
  bool v_17 = (select((v_16 < v_15), 4294967295u, v_16) <= (v_11 / 4u));
  m_array[0u].Store(buffer, (0u + (select(v_17, v_12, 0u) * 4u)), (select(v_17, v_13, 8u) * 4u), MatrixLayout::RowMajor);
  uint v_18 = 0u;
  buffer.GetDimensions(v_18);
  uint v_19 = asuint(int(0));
  uint v_20 = asuint(int(64));
  uint v_21 = (v_19 + select((((v_20 != 0u) & (7u != 0u)) & (7u > (4294967295u / v_20))), 4294967295u, (v_20 * 7u)));
  uint v_22 = select((v_21 < v_19), 4294967295u, v_21);
  uint v_23 = (v_22 + 8u);
  bool v_24 = (select((v_23 < v_22), 4294967295u, v_23) <= (v_18 / 4u));
  m_nested_array[1u][2u].Store(buffer, (0u + (select(v_24, v_19, 0u) * 4u)), (select(v_24, v_20, 8u) * 4u), MatrixLayout::RowMajor);
  uint v_25 = 0u;
  buffer.GetDimensions(v_25);
  uint v_26 = asuint(int(0));
  uint v_27 = asuint(int(64));
  uint v_28 = (v_26 + select((((v_27 != 0u) & (7u != 0u)) & (7u > (4294967295u / v_27))), 4294967295u, (v_27 * 7u)));
  uint v_29 = select((v_28 < v_26), 4294967295u, v_28);
  uint v_30 = (v_29 + 8u);
  bool v_31 = (select((v_30 < v_29), 4294967295u, v_30) <= (v_25 / 4u));
  m_struct.l.Store(buffer, (0u + (select(v_31, v_26, 0u) * 4u)), (select(v_31, v_27, 8u) * 4u), MatrixLayout::RowMajor);
  uint v_32 = 0u;
  buffer.GetDimensions(v_32);
  uint v_33 = asuint(int(0));
  uint v_34 = asuint(int(64));
  uint v_35 = (v_33 + select((((v_34 != 0u) & (7u != 0u)) & (7u > (4294967295u / v_34))), 4294967295u, (v_34 * 7u)));
  uint v_36 = select((v_35 < v_33), 4294967295u, v_35);
  uint v_37 = (v_36 + 8u);
  bool v_38 = (select((v_37 < v_36), 4294967295u, v_37) <= (v_32 / 4u));
  m_nested_struct.s.r.Store(buffer, (0u + (select(v_38, v_33, 0u) * 4u)), (select(v_38, v_34, 8u) * 4u), MatrixLayout::RowMajor);
}

