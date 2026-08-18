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
  Matrix_left_f32_8x8 m_let = m;
  Matrix_left_f32_8x8 m_array_let[4] = m_array;
  Matrix_left_f32_8x8 m_nested_array_let[4][4] = m_nested_array;
  S m_struct_let = m_struct;
  S_Nested m_nested_struct_let = m_nested_struct;
  uint v_4 = 0u;
  buffer.GetDimensions(v_4);
  uint v_5 = asuint(int(0));
  uint v_6 = asuint(int(64));
  bool v_7 = (((v_5 + (v_6 * 7u)) + 8u) <= (v_4 / 4u));
  m_let.Store(buffer, (0u + (select(v_7, v_5, 0u) * 4u)), (select(v_7, v_6, 8u) * 4u), MatrixLayout::RowMajor);
  uint v_8 = 0u;
  buffer.GetDimensions(v_8);
  uint v_9 = asuint(int(0));
  uint v_10 = asuint(int(64));
  bool v_11 = (((v_9 + (v_10 * 7u)) + 8u) <= (v_8 / 4u));
  m_array_let[0u].Store(buffer, (0u + (select(v_11, v_9, 0u) * 4u)), (select(v_11, v_10, 8u) * 4u), MatrixLayout::RowMajor);
  uint v_12 = 0u;
  buffer.GetDimensions(v_12);
  uint v_13 = asuint(int(0));
  uint v_14 = asuint(int(64));
  bool v_15 = (((v_13 + (v_14 * 7u)) + 8u) <= (v_12 / 4u));
  m_nested_array_let[1u][2u].Store(buffer, (0u + (select(v_15, v_13, 0u) * 4u)), (select(v_15, v_14, 8u) * 4u), MatrixLayout::RowMajor);
  uint v_16 = 0u;
  buffer.GetDimensions(v_16);
  uint v_17 = asuint(int(0));
  uint v_18 = asuint(int(64));
  bool v_19 = (((v_17 + (v_18 * 7u)) + 8u) <= (v_16 / 4u));
  m_struct_let.l.Store(buffer, (0u + (select(v_19, v_17, 0u) * 4u)), (select(v_19, v_18, 8u) * 4u), MatrixLayout::RowMajor);
  uint v_20 = 0u;
  buffer.GetDimensions(v_20);
  uint v_21 = asuint(int(0));
  uint v_22 = asuint(int(64));
  bool v_23 = (((v_21 + (v_22 * 7u)) + 8u) <= (v_20 / 4u));
  m_nested_struct_let.s.r.Store(buffer, (0u + (select(v_23, v_21, 0u) * 4u)), (select(v_23, v_22, 8u) * 4u), MatrixLayout::RowMajor);
}

