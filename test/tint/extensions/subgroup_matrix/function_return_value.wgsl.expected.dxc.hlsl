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
  bool v_12 = (((v_10 + (v_11 * 7u)) + 8u) <= (v_9 / 4u));
  v_8.Store(buffer, (0u + (select(v_12, v_10, 0u) * 4u)), (select(v_12, v_11, 8u) * 4u), MatrixLayout::RowMajor);
  Matrix_left_f32_8x8 v_13[4] = make_array();
  Matrix_left_f32_8x8 v_14 = v_13[0u];
  uint v_15 = 0u;
  buffer.GetDimensions(v_15);
  uint v_16 = asuint(int(0));
  uint v_17 = asuint(int(64));
  bool v_18 = (((v_16 + (v_17 * 7u)) + 8u) <= (v_15 / 4u));
  v_14.Store(buffer, (0u + (select(v_18, v_16, 0u) * 4u)), (select(v_18, v_17, 8u) * 4u), MatrixLayout::RowMajor);
  Matrix_left_f32_8x8 v_19[4][4] = make_nested_array();
  Matrix_left_f32_8x8 v_20 = v_19[1u][2u];
  uint v_21 = 0u;
  buffer.GetDimensions(v_21);
  uint v_22 = asuint(int(0));
  uint v_23 = asuint(int(64));
  bool v_24 = (((v_22 + (v_23 * 7u)) + 8u) <= (v_21 / 4u));
  v_20.Store(buffer, (0u + (select(v_24, v_22, 0u) * 4u)), (select(v_24, v_23, 8u) * 4u), MatrixLayout::RowMajor);
  S v_25 = make_struct();
  Matrix_left_f32_8x8 v_26 = v_25.l;
  uint v_27 = 0u;
  buffer.GetDimensions(v_27);
  uint v_28 = asuint(int(0));
  uint v_29 = asuint(int(64));
  bool v_30 = (((v_28 + (v_29 * 7u)) + 8u) <= (v_27 / 4u));
  v_26.Store(buffer, (0u + (select(v_30, v_28, 0u) * 4u)), (select(v_30, v_29, 8u) * 4u), MatrixLayout::RowMajor);
  S_Nested v_31 = make_nested_struct();
  Matrix_right_f32_8x8 v_32 = v_31.s.r;
  uint v_33 = 0u;
  buffer.GetDimensions(v_33);
  uint v_34 = asuint(int(0));
  uint v_35 = asuint(int(64));
  bool v_36 = (((v_34 + (v_35 * 7u)) + 8u) <= (v_33 / 4u));
  v_32.Store(buffer, (0u + (select(v_36, v_34, 0u) * 4u)), (select(v_36, v_35, 8u) * 4u), MatrixLayout::RowMajor);
}

