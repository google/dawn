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
  Matrix_left_f32_8x8 v_4 = m;
  uint v_5 = 0u;
  buffer.GetDimensions(v_5);
  if ((((asuint(int(0)) + (asuint(int(64)) * 7u)) + 8u) <= (v_5 / 4u))) {
    v_4.Store(buffer, 0u, 256u, MatrixLayout::RowMajor);
  }
  Matrix_left_f32_8x8 v_6 = m_array[0u];
  uint v_7 = 0u;
  buffer.GetDimensions(v_7);
  if ((((asuint(int(0)) + (asuint(int(64)) * 7u)) + 8u) <= (v_7 / 4u))) {
    v_6.Store(buffer, 0u, 256u, MatrixLayout::RowMajor);
  }
  Matrix_left_f32_8x8 v_8 = m_nested_array[1u][2u];
  uint v_9 = 0u;
  buffer.GetDimensions(v_9);
  if ((((asuint(int(0)) + (asuint(int(64)) * 7u)) + 8u) <= (v_9 / 4u))) {
    v_8.Store(buffer, 0u, 256u, MatrixLayout::RowMajor);
  }
  Matrix_left_f32_8x8 v_10 = m_struct.l;
  uint v_11 = 0u;
  buffer.GetDimensions(v_11);
  if ((((asuint(int(0)) + (asuint(int(64)) * 7u)) + 8u) <= (v_11 / 4u))) {
    v_10.Store(buffer, 0u, 256u, MatrixLayout::RowMajor);
  }
  Matrix_right_f32_8x8 v_12 = m_nested_struct.s.r;
  uint v_13 = 0u;
  buffer.GetDimensions(v_13);
  if ((((asuint(int(0)) + (asuint(int(64)) * 7u)) + 8u) <= (v_13 / 4u))) {
    v_12.Store(buffer, 0u, 256u, MatrixLayout::RowMajor);
  }
}

