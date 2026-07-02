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
void foo(inout Matrix_left_f32_8x8 m, inout Matrix_left_f32_8x8 m_array[4], inout Matrix_left_f32_8x8 m_nested_array[4][4], inout S m_struct, inout S_Nested m_nested_struct) {
  Matrix_left_f32_8x8 v = m;
  uint v_1 = 0u;
  buffer.GetDimensions(v_1);
  if ((((asuint(int(0)) + (asuint(int(64)) * 7u)) + 8u) <= (v_1 / 4u))) {
    v.Store(buffer, 0u, 256u, MatrixLayout::RowMajor);
  }
  Matrix_left_f32_8x8 v_2 = m_array[0u];
  uint v_3 = 0u;
  buffer.GetDimensions(v_3);
  if ((((asuint(int(0)) + (asuint(int(64)) * 7u)) + 8u) <= (v_3 / 4u))) {
    v_2.Store(buffer, 0u, 256u, MatrixLayout::RowMajor);
  }
  Matrix_left_f32_8x8 v_4 = m_nested_array[1u][2u];
  uint v_5 = 0u;
  buffer.GetDimensions(v_5);
  if ((((asuint(int(0)) + (asuint(int(64)) * 7u)) + 8u) <= (v_5 / 4u))) {
    v_4.Store(buffer, 0u, 256u, MatrixLayout::RowMajor);
  }
  Matrix_left_f32_8x8 v_6 = m_struct.l;
  uint v_7 = 0u;
  buffer.GetDimensions(v_7);
  if ((((asuint(int(0)) + (asuint(int(64)) * 7u)) + 8u) <= (v_7 / 4u))) {
    v_6.Store(buffer, 0u, 256u, MatrixLayout::RowMajor);
  }
  Matrix_right_f32_8x8 v_8 = m_nested_struct.s.r;
  uint v_9 = 0u;
  buffer.GetDimensions(v_9);
  if ((((asuint(int(0)) + (asuint(int(64)) * 7u)) + 8u) <= (v_9 / 4u))) {
    v_8.Store(buffer, 0u, 256u, MatrixLayout::RowMajor);
  }
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
  foo(m, m_array, m_nested_array, m_struct, m_nested_struct);
}

