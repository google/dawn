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
  uint v_1 = asuint(int(0));
  uint v_2 = asuint(int(64));
  bool v_3 = (((v_1 + (v_2 * 7u)) + 8u) <= (v / 4u));
  Matrix_left_f32_8x8::Splat(0.0f).Store(buffer, (0u + (select(v_3, v_1, 0u) * 4u)), (select(v_3, v_2, 8u) * 4u), MatrixLayout::RowMajor);
  Matrix_left_f32_8x8 v_4 = Matrix_left_f32_8x8::Splat(0.0f);
  Matrix_left_f32_8x8 v_5[4] = {v_4, v_4, v_4, v_4};
  Matrix_left_f32_8x8 v_6 = v_5[1u];
  uint v_7 = 0u;
  buffer.GetDimensions(v_7);
  uint v_8 = asuint(int(0));
  uint v_9 = asuint(int(64));
  bool v_10 = (((v_8 + (v_9 * 7u)) + 8u) <= (v_7 / 4u));
  v_6.Store(buffer, (0u + (select(v_10, v_8, 0u) * 4u)), (select(v_10, v_9, 8u) * 4u), MatrixLayout::RowMajor);
  Matrix_left_f32_8x8 v_11 = Matrix_left_f32_8x8::Splat(0.0f);
  Matrix_left_f32_8x8 v_12[4] = {v_11, v_11, v_11, v_11};
  Matrix_left_f32_8x8 v_13[4][4] = {v_12, v_12, v_12, v_12};
  Matrix_left_f32_8x8 v_14 = v_13[2u][3u];
  uint v_15 = 0u;
  buffer.GetDimensions(v_15);
  uint v_16 = asuint(int(0));
  uint v_17 = asuint(int(64));
  bool v_18 = (((v_16 + (v_17 * 7u)) + 8u) <= (v_15 / 4u));
  v_14.Store(buffer, (0u + (select(v_18, v_16, 0u) * 4u)), (select(v_18, v_17, 8u) * 4u), MatrixLayout::RowMajor);
  S v_19 = {Matrix_left_f32_8x8::Splat(0.0f), Matrix_right_f32_8x8::Splat(0.0f)};
  Matrix_left_f32_8x8 v_20 = v_19.l;
  uint v_21 = 0u;
  buffer.GetDimensions(v_21);
  uint v_22 = asuint(int(0));
  uint v_23 = asuint(int(64));
  bool v_24 = (((v_22 + (v_23 * 7u)) + 8u) <= (v_21 / 4u));
  v_20.Store(buffer, (0u + (select(v_24, v_22, 0u) * 4u)), (select(v_24, v_23, 8u) * 4u), MatrixLayout::RowMajor);
  S v_25 = {Matrix_left_f32_8x8::Splat(0.0f), Matrix_right_f32_8x8::Splat(0.0f)};
  S_Nested v_26 = {v_25};
  Matrix_right_f32_8x8 v_27 = v_26.s.r;
  uint v_28 = 0u;
  buffer.GetDimensions(v_28);
  uint v_29 = asuint(int(0));
  uint v_30 = asuint(int(64));
  bool v_31 = (((v_29 + (v_30 * 7u)) + 8u) <= (v_28 / 4u));
  v_27.Store(buffer, (0u + (select(v_31, v_29, 0u) * 4u)), (select(v_31, v_30, 8u) * 4u), MatrixLayout::RowMajor);
  uint v_32 = 0u;
  buffer.GetDimensions(v_32);
  uint v_33 = asuint(int(0));
  uint v_34 = asuint(int(64));
  bool v_35 = (((v_33 + (v_34 * 7u)) + 8u) <= (v_32 / 4u));
  Matrix_left_f32_8x8::Splat(42.0f).Store(buffer, (0u + (select(v_35, v_33, 0u) * 4u)), (select(v_35, v_34, 8u) * 4u), MatrixLayout::RowMajor);
  Matrix_left_f32_8x8 v_36[2] = {Matrix_left_f32_8x8::Splat(42.0f), Matrix_left_f32_8x8::Splat(100.0f)};
  Matrix_left_f32_8x8 v_37 = v_36[1u];
  uint v_38 = 0u;
  buffer.GetDimensions(v_38);
  uint v_39 = asuint(int(0));
  uint v_40 = asuint(int(64));
  bool v_41 = (((v_39 + (v_40 * 7u)) + 8u) <= (v_38 / 4u));
  v_37.Store(buffer, (0u + (select(v_41, v_39, 0u) * 4u)), (select(v_41, v_40, 8u) * 4u), MatrixLayout::RowMajor);
  Matrix_left_f32_8x8 v_42[2] = {Matrix_left_f32_8x8::Splat(42.0f), Matrix_left_f32_8x8::Splat(100.0f)};
  Matrix_left_f32_8x8 v_43[2] = {Matrix_left_f32_8x8::Splat(-7.0f), Matrix_left_f32_8x8::Splat(-42.0f)};
  Matrix_left_f32_8x8 v_44[2][2] = {v_42, v_43};
  Matrix_left_f32_8x8 v_45 = v_44[1u][0u];
  uint v_46 = 0u;
  buffer.GetDimensions(v_46);
  uint v_47 = asuint(int(0));
  uint v_48 = asuint(int(64));
  bool v_49 = (((v_47 + (v_48 * 7u)) + 8u) <= (v_46 / 4u));
  v_45.Store(buffer, (0u + (select(v_49, v_47, 0u) * 4u)), (select(v_49, v_48, 8u) * 4u), MatrixLayout::RowMajor);
  S v_50 = {Matrix_left_f32_8x8::Splat(42.0f), Matrix_right_f32_8x8::Splat(100.0f)};
  Matrix_left_f32_8x8 v_51 = v_50.l;
  uint v_52 = 0u;
  buffer.GetDimensions(v_52);
  uint v_53 = asuint(int(0));
  uint v_54 = asuint(int(64));
  bool v_55 = (((v_53 + (v_54 * 7u)) + 8u) <= (v_52 / 4u));
  v_51.Store(buffer, (0u + (select(v_55, v_53, 0u) * 4u)), (select(v_55, v_54, 8u) * 4u), MatrixLayout::RowMajor);
  S v_56 = {Matrix_left_f32_8x8::Splat(42.0f), Matrix_right_f32_8x8::Splat(100.0f)};
  S_Nested v_57 = {v_56};
  Matrix_right_f32_8x8 v_58 = v_57.s.r;
  uint v_59 = 0u;
  buffer.GetDimensions(v_59);
  uint v_60 = asuint(int(0));
  uint v_61 = asuint(int(64));
  bool v_62 = (((v_60 + (v_61 * 7u)) + 8u) <= (v_59 / 4u));
  v_58.Store(buffer, (0u + (select(v_62, v_60, 0u) * 4u)), (select(v_62, v_61, 8u) * 4u), MatrixLayout::RowMajor);
}

