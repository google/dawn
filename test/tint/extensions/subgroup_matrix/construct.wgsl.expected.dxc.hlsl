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
  bool v_1 = (456u <= (v / 4u));
  Matrix_left_f32_8x8::Splat(0.0f).Store(buffer, (0u + (select(v_1, 0u, 0u) * 4u)), (select(v_1, 64u, 8u) * 4u), MatrixLayout::RowMajor);
  Matrix_left_f32_8x8 v_2 = Matrix_left_f32_8x8::Splat(0.0f);
  Matrix_left_f32_8x8 v_3[4] = {v_2, v_2, v_2, v_2};
  Matrix_left_f32_8x8 v_4 = v_3[1u];
  uint v_5 = 0u;
  buffer.GetDimensions(v_5);
  bool v_6 = (456u <= (v_5 / 4u));
  v_4.Store(buffer, (0u + (select(v_6, 0u, 0u) * 4u)), (select(v_6, 64u, 8u) * 4u), MatrixLayout::RowMajor);
  Matrix_left_f32_8x8 v_7 = Matrix_left_f32_8x8::Splat(0.0f);
  Matrix_left_f32_8x8 v_8[4] = {v_7, v_7, v_7, v_7};
  Matrix_left_f32_8x8 v_9[4][4] = {v_8, v_8, v_8, v_8};
  Matrix_left_f32_8x8 v_10 = v_9[2u][3u];
  uint v_11 = 0u;
  buffer.GetDimensions(v_11);
  bool v_12 = (456u <= (v_11 / 4u));
  v_10.Store(buffer, (0u + (select(v_12, 0u, 0u) * 4u)), (select(v_12, 64u, 8u) * 4u), MatrixLayout::RowMajor);
  S v_13 = {Matrix_left_f32_8x8::Splat(0.0f), Matrix_right_f32_8x8::Splat(0.0f)};
  Matrix_left_f32_8x8 v_14 = v_13.l;
  uint v_15 = 0u;
  buffer.GetDimensions(v_15);
  bool v_16 = (456u <= (v_15 / 4u));
  v_14.Store(buffer, (0u + (select(v_16, 0u, 0u) * 4u)), (select(v_16, 64u, 8u) * 4u), MatrixLayout::RowMajor);
  S v_17 = {Matrix_left_f32_8x8::Splat(0.0f), Matrix_right_f32_8x8::Splat(0.0f)};
  S_Nested v_18 = {v_17};
  Matrix_right_f32_8x8 v_19 = v_18.s.r;
  uint v_20 = 0u;
  buffer.GetDimensions(v_20);
  bool v_21 = (456u <= (v_20 / 4u));
  v_19.Store(buffer, (0u + (select(v_21, 0u, 0u) * 4u)), (select(v_21, 64u, 8u) * 4u), MatrixLayout::RowMajor);
  uint v_22 = 0u;
  buffer.GetDimensions(v_22);
  bool v_23 = (456u <= (v_22 / 4u));
  Matrix_left_f32_8x8::Splat(42.0f).Store(buffer, (0u + (select(v_23, 0u, 0u) * 4u)), (select(v_23, 64u, 8u) * 4u), MatrixLayout::RowMajor);
  Matrix_left_f32_8x8 v_24[2] = {Matrix_left_f32_8x8::Splat(42.0f), Matrix_left_f32_8x8::Splat(100.0f)};
  Matrix_left_f32_8x8 v_25 = v_24[1u];
  uint v_26 = 0u;
  buffer.GetDimensions(v_26);
  bool v_27 = (456u <= (v_26 / 4u));
  v_25.Store(buffer, (0u + (select(v_27, 0u, 0u) * 4u)), (select(v_27, 64u, 8u) * 4u), MatrixLayout::RowMajor);
  Matrix_left_f32_8x8 v_28[2] = {Matrix_left_f32_8x8::Splat(42.0f), Matrix_left_f32_8x8::Splat(100.0f)};
  Matrix_left_f32_8x8 v_29[2] = {Matrix_left_f32_8x8::Splat(-7.0f), Matrix_left_f32_8x8::Splat(-42.0f)};
  Matrix_left_f32_8x8 v_30[2][2] = {v_28, v_29};
  Matrix_left_f32_8x8 v_31 = v_30[1u][0u];
  uint v_32 = 0u;
  buffer.GetDimensions(v_32);
  bool v_33 = (456u <= (v_32 / 4u));
  v_31.Store(buffer, (0u + (select(v_33, 0u, 0u) * 4u)), (select(v_33, 64u, 8u) * 4u), MatrixLayout::RowMajor);
  S v_34 = {Matrix_left_f32_8x8::Splat(42.0f), Matrix_right_f32_8x8::Splat(100.0f)};
  Matrix_left_f32_8x8 v_35 = v_34.l;
  uint v_36 = 0u;
  buffer.GetDimensions(v_36);
  bool v_37 = (456u <= (v_36 / 4u));
  v_35.Store(buffer, (0u + (select(v_37, 0u, 0u) * 4u)), (select(v_37, 64u, 8u) * 4u), MatrixLayout::RowMajor);
  S v_38 = {Matrix_left_f32_8x8::Splat(42.0f), Matrix_right_f32_8x8::Splat(100.0f)};
  S_Nested v_39 = {v_38};
  Matrix_right_f32_8x8 v_40 = v_39.s.r;
  uint v_41 = 0u;
  buffer.GetDimensions(v_41);
  bool v_42 = (456u <= (v_41 / 4u));
  v_40.Store(buffer, (0u + (select(v_42, 0u, 0u) * 4u)), (select(v_42, 64u, 8u) * 4u), MatrixLayout::RowMajor);
}

