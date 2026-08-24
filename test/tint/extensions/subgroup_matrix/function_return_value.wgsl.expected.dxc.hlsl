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
Matrix_left_f32_8x8 make_matrix() {
  Matrix_left_f32_8x8 m = Matrix_left_f32_8x8::Splat(0.0f);
  return m;
}

typedef Matrix_left_f32_8x8 ary_ret[4];
ary_ret make_array() {
  Matrix_left_f32_8x8 v = Matrix_left_f32_8x8::Splat(0.0f);
  Matrix_left_f32_8x8 m_array[4] = {v, v, v, v};
  Matrix_left_f32_8x8 v_1[4] = m_array;
  return v_1;
}

typedef Matrix_left_f32_8x8 ary_ret_1[4][4];
ary_ret_1 make_nested_array() {
  Matrix_left_f32_8x8 v_2 = Matrix_left_f32_8x8::Splat(0.0f);
  Matrix_left_f32_8x8 v_3[4] = {v_2, v_2, v_2, v_2};
  Matrix_left_f32_8x8 m_nested_array[4][4] = {v_3, v_3, v_3, v_3};
  Matrix_left_f32_8x8 v_4[4][4] = m_nested_array;
  return v_4;
}

S make_struct() {
  S m_struct = {Matrix_left_f32_8x8::Splat(0.0f), Matrix_right_f32_8x8::Splat(0.0f)};
  S v_5 = m_struct;
  return v_5;
}

S_Nested make_nested_struct() {
  S v_6 = {Matrix_left_f32_8x8::Splat(0.0f), Matrix_right_f32_8x8::Splat(0.0f)};
  S_Nested m_nested_struct = {v_6};
  S_Nested v_7 = m_nested_struct;
  return v_7;
}

[numthreads(64, 1, 1)]
void main() {
  Matrix_left_f32_8x8 v_8 = make_matrix();
  uint v_9 = 0u;
  buffer.GetDimensions(v_9);
  uint v_10 = asuint(int(0));
  uint v_11 = asuint(int(64));
  uint v_12 = (v_10 + select((((v_11 != 0u) & (7u != 0u)) & (7u > (4294967295u / v_11))), 4294967295u, (v_11 * 7u)));
  uint v_13 = select((v_12 < v_10), 4294967295u, v_12);
  uint v_14 = (v_13 + 8u);
  bool v_15 = (select((v_14 < v_13), 4294967295u, v_14) <= (v_9 / 4u));
  v_8.Store(buffer, (0u + (select(v_15, v_10, 0u) * 4u)), (select(v_15, v_11, 8u) * 4u), MatrixLayout::RowMajor);
  Matrix_left_f32_8x8 v_16[4] = make_array();
  Matrix_left_f32_8x8 v_17 = v_16[0u];
  uint v_18 = 0u;
  buffer.GetDimensions(v_18);
  uint v_19 = asuint(int(0));
  uint v_20 = asuint(int(64));
  uint v_21 = (v_19 + select((((v_20 != 0u) & (7u != 0u)) & (7u > (4294967295u / v_20))), 4294967295u, (v_20 * 7u)));
  uint v_22 = select((v_21 < v_19), 4294967295u, v_21);
  uint v_23 = (v_22 + 8u);
  bool v_24 = (select((v_23 < v_22), 4294967295u, v_23) <= (v_18 / 4u));
  v_17.Store(buffer, (0u + (select(v_24, v_19, 0u) * 4u)), (select(v_24, v_20, 8u) * 4u), MatrixLayout::RowMajor);
  Matrix_left_f32_8x8 v_25[4][4] = make_nested_array();
  Matrix_left_f32_8x8 v_26 = v_25[1u][2u];
  uint v_27 = 0u;
  buffer.GetDimensions(v_27);
  uint v_28 = asuint(int(0));
  uint v_29 = asuint(int(64));
  uint v_30 = (v_28 + select((((v_29 != 0u) & (7u != 0u)) & (7u > (4294967295u / v_29))), 4294967295u, (v_29 * 7u)));
  uint v_31 = select((v_30 < v_28), 4294967295u, v_30);
  uint v_32 = (v_31 + 8u);
  bool v_33 = (select((v_32 < v_31), 4294967295u, v_32) <= (v_27 / 4u));
  v_26.Store(buffer, (0u + (select(v_33, v_28, 0u) * 4u)), (select(v_33, v_29, 8u) * 4u), MatrixLayout::RowMajor);
  S v_34 = make_struct();
  Matrix_left_f32_8x8 v_35 = v_34.l;
  uint v_36 = 0u;
  buffer.GetDimensions(v_36);
  uint v_37 = asuint(int(0));
  uint v_38 = asuint(int(64));
  uint v_39 = (v_37 + select((((v_38 != 0u) & (7u != 0u)) & (7u > (4294967295u / v_38))), 4294967295u, (v_38 * 7u)));
  uint v_40 = select((v_39 < v_37), 4294967295u, v_39);
  uint v_41 = (v_40 + 8u);
  bool v_42 = (select((v_41 < v_40), 4294967295u, v_41) <= (v_36 / 4u));
  v_35.Store(buffer, (0u + (select(v_42, v_37, 0u) * 4u)), (select(v_42, v_38, 8u) * 4u), MatrixLayout::RowMajor);
  S_Nested v_43 = make_nested_struct();
  Matrix_right_f32_8x8 v_44 = v_43.s.r;
  uint v_45 = 0u;
  buffer.GetDimensions(v_45);
  uint v_46 = asuint(int(0));
  uint v_47 = asuint(int(64));
  uint v_48 = (v_46 + select((((v_47 != 0u) & (7u != 0u)) & (7u > (4294967295u / v_47))), 4294967295u, (v_47 * 7u)));
  uint v_49 = select((v_48 < v_46), 4294967295u, v_48);
  uint v_50 = (v_49 + 8u);
  bool v_51 = (select((v_50 < v_49), 4294967295u, v_50) <= (v_45 / 4u));
  v_44.Store(buffer, (0u + (select(v_51, v_46, 0u) * 4u)), (select(v_51, v_47, 8u) * 4u), MatrixLayout::RowMajor);
}

