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
  bool v_5 = (((0u + (64u * 7u)) + 8u) <= (v_4 / 4u));
  m.Store(buffer, (0u + (select(v_5, 0u, 0u) * 4u)), (select(v_5, 64u, 8u) * 4u), MatrixLayout::RowMajor);
  uint v_6 = 0u;
  buffer.GetDimensions(v_6);
  bool v_7 = (((0u + (64u * 7u)) + 8u) <= (v_6 / 4u));
  m_array[0u].Store(buffer, (0u + (select(v_7, 0u, 0u) * 4u)), (select(v_7, 64u, 8u) * 4u), MatrixLayout::RowMajor);
  uint v_8 = 0u;
  buffer.GetDimensions(v_8);
  bool v_9 = (((0u + (64u * 7u)) + 8u) <= (v_8 / 4u));
  m_nested_array[1u][2u].Store(buffer, (0u + (select(v_9, 0u, 0u) * 4u)), (select(v_9, 64u, 8u) * 4u), MatrixLayout::RowMajor);
  uint v_10 = 0u;
  buffer.GetDimensions(v_10);
  bool v_11 = (((0u + (64u * 7u)) + 8u) <= (v_10 / 4u));
  m_struct.l.Store(buffer, (0u + (select(v_11, 0u, 0u) * 4u)), (select(v_11, 64u, 8u) * 4u), MatrixLayout::RowMajor);
  uint v_12 = 0u;
  buffer.GetDimensions(v_12);
  bool v_13 = (((0u + (64u * 7u)) + 8u) <= (v_12 / 4u));
  m_nested_struct.s.r.Store(buffer, (0u + (select(v_13, 0u, 0u) * 4u)), (select(v_13, 64u, 8u) * 4u), MatrixLayout::RowMajor);
}

