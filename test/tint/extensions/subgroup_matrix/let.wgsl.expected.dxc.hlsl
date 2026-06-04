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
  if ((((0u + (64u * 7u)) + 8u) <= (v_4 / 4u))) {
    m_let.Store(buffer, 0u, 256u, MatrixLayout::RowMajor);
  }
  uint v_5 = 0u;
  buffer.GetDimensions(v_5);
  if ((((0u + (64u * 7u)) + 8u) <= (v_5 / 4u))) {
    m_array_let[0u].Store(buffer, 0u, 256u, MatrixLayout::RowMajor);
  }
  uint v_6 = 0u;
  buffer.GetDimensions(v_6);
  if ((((0u + (64u * 7u)) + 8u) <= (v_6 / 4u))) {
    m_nested_array_let[1u][2u].Store(buffer, 0u, 256u, MatrixLayout::RowMajor);
  }
  uint v_7 = 0u;
  buffer.GetDimensions(v_7);
  if ((((0u + (64u * 7u)) + 8u) <= (v_7 / 4u))) {
    m_struct_let.l.Store(buffer, 0u, 256u, MatrixLayout::RowMajor);
  }
  uint v_8 = 0u;
  buffer.GetDimensions(v_8);
  if ((((0u + (64u * 7u)) + 8u) <= (v_8 / 4u))) {
    m_nested_struct_let.s.r.Store(buffer, 0u, 256u, MatrixLayout::RowMajor);
  }
}

