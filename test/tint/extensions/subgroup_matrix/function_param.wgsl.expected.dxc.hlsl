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
  if ((((asuint(int(0)) + (asuint(int(64)) * 7u)) + 8u) <= (v / 4u))) {
    m.Store(buffer, 0u, 256u, MatrixLayout::RowMajor);
  }
  uint v_1 = 0u;
  buffer.GetDimensions(v_1);
  if ((((asuint(int(0)) + (asuint(int(64)) * 7u)) + 8u) <= (v_1 / 4u))) {
    m_array[0u].Store(buffer, 0u, 256u, MatrixLayout::RowMajor);
  }
  uint v_2 = 0u;
  buffer.GetDimensions(v_2);
  if ((((asuint(int(0)) + (asuint(int(64)) * 7u)) + 8u) <= (v_2 / 4u))) {
    m_nested_array[1u][2u].Store(buffer, 0u, 256u, MatrixLayout::RowMajor);
  }
  uint v_3 = 0u;
  buffer.GetDimensions(v_3);
  if ((((asuint(int(0)) + (asuint(int(64)) * 7u)) + 8u) <= (v_3 / 4u))) {
    m_struct.l.Store(buffer, 0u, 256u, MatrixLayout::RowMajor);
  }
  uint v_4 = 0u;
  buffer.GetDimensions(v_4);
  if ((((asuint(int(0)) + (asuint(int(64)) * 7u)) + 8u) <= (v_4 / 4u))) {
    m_nested_struct.s.r.Store(buffer, 0u, 256u, MatrixLayout::RowMajor);
  }
}

[numthreads(64, 1, 1)]
void main() {
  Matrix_left_f32_8x8 m = Matrix_left_f32_8x8::Splat(0.0f);
  Matrix_left_f32_8x8 v_5 = Matrix_left_f32_8x8::Splat(0.0f);
  Matrix_left_f32_8x8 m_array[4] = {v_5, v_5, v_5, v_5};
  Matrix_left_f32_8x8 v_6 = Matrix_left_f32_8x8::Splat(0.0f);
  Matrix_left_f32_8x8 v_7[4] = {v_6, v_6, v_6, v_6};
  Matrix_left_f32_8x8 m_nested_array[4][4] = {v_7, v_7, v_7, v_7};
  S m_struct = {Matrix_left_f32_8x8::Splat(0.0f), Matrix_right_f32_8x8::Splat(0.0f)};
  S v_8 = {Matrix_left_f32_8x8::Splat(0.0f), Matrix_right_f32_8x8::Splat(0.0f)};
  S_Nested m_nested_struct = {v_8};
  Matrix_left_f32_8x8 v_9[4] = m_array;
  Matrix_left_f32_8x8 v_10[4][4] = m_nested_array;
  S v_11 = m_struct;
  S_Nested v_12 = m_nested_struct;
  foo(m, v_9, v_10, v_11, v_12);
}

