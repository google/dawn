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
  uint v = 0u;
  buffer.GetDimensions(v);
  if ((((0u + (64u * 7u)) + 8u) <= (v / 4u))) {
    Matrix_left_f32_8x8::Splat(0.0f).Store(buffer, 0u, 256u, MatrixLayout::RowMajor);
  }
  Matrix_left_f32_8x8 v_1 = Matrix_left_f32_8x8::Splat(0.0f);
  Matrix_left_f32_8x8 v_2[4] = {v_1, v_1, v_1, v_1};
  Matrix_left_f32_8x8 v_3 = v_2[1u];
  uint v_4 = 0u;
  buffer.GetDimensions(v_4);
  if ((((0u + (64u * 7u)) + 8u) <= (v_4 / 4u))) {
    v_3.Store(buffer, 0u, 256u, MatrixLayout::RowMajor);
  }
  Matrix_left_f32_8x8 v_5 = Matrix_left_f32_8x8::Splat(0.0f);
  Matrix_left_f32_8x8 v_6[4] = {v_5, v_5, v_5, v_5};
  Matrix_left_f32_8x8 v_7[4][4] = {v_6, v_6, v_6, v_6};
  Matrix_left_f32_8x8 v_8 = v_7[2u][3u];
  uint v_9 = 0u;
  buffer.GetDimensions(v_9);
  if ((((0u + (64u * 7u)) + 8u) <= (v_9 / 4u))) {
    v_8.Store(buffer, 0u, 256u, MatrixLayout::RowMajor);
  }
  S v_10 = {Matrix_left_f32_8x8::Splat(0.0f), Matrix_right_f32_8x8::Splat(0.0f)};
  Matrix_left_f32_8x8 v_11 = v_10.l;
  uint v_12 = 0u;
  buffer.GetDimensions(v_12);
  if ((((0u + (64u * 7u)) + 8u) <= (v_12 / 4u))) {
    v_11.Store(buffer, 0u, 256u, MatrixLayout::RowMajor);
  }
  S v_13 = {Matrix_left_f32_8x8::Splat(0.0f), Matrix_right_f32_8x8::Splat(0.0f)};
  S_Nested v_14 = {v_13};
  Matrix_right_f32_8x8 v_15 = v_14.s.r;
  uint v_16 = 0u;
  buffer.GetDimensions(v_16);
  if ((((0u + (64u * 7u)) + 8u) <= (v_16 / 4u))) {
    v_15.Store(buffer, 0u, 256u, MatrixLayout::RowMajor);
  }
  uint v_17 = 0u;
  buffer.GetDimensions(v_17);
  if ((((0u + (64u * 7u)) + 8u) <= (v_17 / 4u))) {
    Matrix_left_f32_8x8::Splat(42.0f).Store(buffer, 0u, 256u, MatrixLayout::RowMajor);
  }
  Matrix_left_f32_8x8 v_18[2] = {Matrix_left_f32_8x8::Splat(42.0f), Matrix_left_f32_8x8::Splat(100.0f)};
  Matrix_left_f32_8x8 v_19 = v_18[1u];
  uint v_20 = 0u;
  buffer.GetDimensions(v_20);
  if ((((0u + (64u * 7u)) + 8u) <= (v_20 / 4u))) {
    v_19.Store(buffer, 0u, 256u, MatrixLayout::RowMajor);
  }
  Matrix_left_f32_8x8 v_21[2] = {Matrix_left_f32_8x8::Splat(42.0f), Matrix_left_f32_8x8::Splat(100.0f)};
  Matrix_left_f32_8x8 v_22[2] = {Matrix_left_f32_8x8::Splat(-7.0f), Matrix_left_f32_8x8::Splat(-42.0f)};
  Matrix_left_f32_8x8 v_23[2][2] = {v_21, v_22};
  Matrix_left_f32_8x8 v_24 = v_23[1u][0u];
  uint v_25 = 0u;
  buffer.GetDimensions(v_25);
  if ((((0u + (64u * 7u)) + 8u) <= (v_25 / 4u))) {
    v_24.Store(buffer, 0u, 256u, MatrixLayout::RowMajor);
  }
  S v_26 = {Matrix_left_f32_8x8::Splat(42.0f), Matrix_right_f32_8x8::Splat(100.0f)};
  Matrix_left_f32_8x8 v_27 = v_26.l;
  uint v_28 = 0u;
  buffer.GetDimensions(v_28);
  if ((((0u + (64u * 7u)) + 8u) <= (v_28 / 4u))) {
    v_27.Store(buffer, 0u, 256u, MatrixLayout::RowMajor);
  }
  S v_29 = {Matrix_left_f32_8x8::Splat(42.0f), Matrix_right_f32_8x8::Splat(100.0f)};
  S_Nested v_30 = {v_29};
  Matrix_right_f32_8x8 v_31 = v_30.s.r;
  uint v_32 = 0u;
  buffer.GetDimensions(v_32);
  if ((((0u + (64u * 7u)) + 8u) <= (v_32 / 4u))) {
    v_31.Store(buffer, 0u, 256u, MatrixLayout::RowMajor);
  }
}

