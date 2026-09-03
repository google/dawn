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
void foo(Matrix_left_f32_8x8 m, Matrix_left_f32_8x8 m_array[4], Matrix_left_f32_8x8 m_nested_array[4][4], S m_struct, S_Nested m_nested_struct) {
  uint v = 0u;
  buffer.GetDimensions(v);
  bool v_1 = (456u <= (v / 4u));
  m.Store(buffer, (0u + (select(v_1, 0u, 0u) * 4u)), (select(v_1, 64u, 8u) * 4u), MatrixLayout::RowMajor);
  uint v_2 = 0u;
  buffer.GetDimensions(v_2);
  bool v_3 = (456u <= (v_2 / 4u));
  m_array[int(0)].Store(buffer, (0u + (select(v_3, 0u, 0u) * 4u)), (select(v_3, 64u, 8u) * 4u), MatrixLayout::RowMajor);
  uint v_4 = 0u;
  buffer.GetDimensions(v_4);
  bool v_5 = (456u <= (v_4 / 4u));
  m_nested_array[int(1)][int(2)].Store(buffer, (0u + (select(v_5, 0u, 0u) * 4u)), (select(v_5, 64u, 8u) * 4u), MatrixLayout::RowMajor);
  uint v_6 = 0u;
  buffer.GetDimensions(v_6);
  bool v_7 = (456u <= (v_6 / 4u));
  m_struct.l.Store(buffer, (0u + (select(v_7, 0u, 0u) * 4u)), (select(v_7, 64u, 8u) * 4u), MatrixLayout::RowMajor);
  uint v_8 = 0u;
  buffer.GetDimensions(v_8);
  bool v_9 = (456u <= (v_8 / 4u));
  m_nested_struct.s.r.Store(buffer, (0u + (select(v_9, 0u, 0u) * 4u)), (select(v_9, 64u, 8u) * 4u), MatrixLayout::RowMajor);
}

[numthreads(64, 1, 1)]
void main() {
  Matrix_left_f32_8x8 m = Matrix_left_f32_8x8::Splat(0.0f);
  Matrix_left_f32_8x8 v_10 = Matrix_left_f32_8x8::Splat(0.0f);
  Matrix_left_f32_8x8 m_array[4] = {v_10, v_10, v_10, v_10};
  Matrix_left_f32_8x8 v_11 = Matrix_left_f32_8x8::Splat(0.0f);
  Matrix_left_f32_8x8 v_12[4] = {v_11, v_11, v_11, v_11};
  Matrix_left_f32_8x8 m_nested_array[4][4] = {v_12, v_12, v_12, v_12};
  S m_struct = {Matrix_left_f32_8x8::Splat(0.0f), Matrix_right_f32_8x8::Splat(0.0f)};
  S v_13 = {Matrix_left_f32_8x8::Splat(0.0f), Matrix_right_f32_8x8::Splat(0.0f)};
  S_Nested m_nested_struct = {v_13};
  Matrix_left_f32_8x8 v_14[4] = m_array;
  Matrix_left_f32_8x8 v_15[4][4] = m_nested_array;
  S v_16 = m_struct;
  S_Nested v_17 = m_nested_struct;
  foo(m, v_14, v_15, v_16, v_17);
}

