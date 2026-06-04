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

struct main_inputs {
  uint idx : SV_GroupIndex;
};


RWByteAddressBuffer buffer : register(u0);
static bool non_uniform_condition = false;
Matrix_left_f32_8x8 make_matrix() {
  if (non_uniform_condition) {
    return Matrix_left_f32_8x8::Splat(1.0f);
  } else {
    return Matrix_left_f32_8x8::Splat(2.0f);
  }
  return Matrix_left_f32_8x8::Splat(0.0f);
}

typedef Matrix_left_f32_8x8 ary_ret[2];
ary_ret make_array() {
  if (non_uniform_condition) {
    Matrix_left_f32_8x8 v[2] = {Matrix_left_f32_8x8::Splat(42.0f), Matrix_left_f32_8x8::Splat(100.0f)};
    return v;
  } else {
    Matrix_left_f32_8x8 v_1[2] = {Matrix_left_f32_8x8::Splat(-7.0f), Matrix_left_f32_8x8::Splat(-42.0f)};
    return v_1;
  }
  Matrix_left_f32_8x8 v_2 = Matrix_left_f32_8x8::Splat(0.0f);
  Matrix_left_f32_8x8 v_3[2] = {v_2, v_2};
  return v_3;
}

typedef Matrix_left_f32_8x8 ary_ret_1[2][2];
ary_ret_1 make_nested_array() {
  Matrix_left_f32_8x8 v_4 = Matrix_left_f32_8x8::Splat(0.0f);
  Matrix_left_f32_8x8 v_5[2] = {v_4, v_4};
  Matrix_left_f32_8x8 a[2][2] = {v_5, v_5};
  Matrix_left_f32_8x8 v_6 = Matrix_left_f32_8x8::Splat(0.0f);
  Matrix_left_f32_8x8 v_7[2] = {v_6, v_6};
  Matrix_left_f32_8x8 b[2][2] = {v_7, v_7};
  if (non_uniform_condition) {
    Matrix_left_f32_8x8 v_8[2] = {Matrix_left_f32_8x8::Splat(42.0f), Matrix_left_f32_8x8::Splat(100.0f)};
    Matrix_left_f32_8x8 v_9[2] = {Matrix_left_f32_8x8::Splat(-7.0f), Matrix_left_f32_8x8::Splat(-42.0f)};
    Matrix_left_f32_8x8 v_10[2][2] = {v_8, v_9};
    return v_10;
  } else {
    Matrix_left_f32_8x8 v_11[2] = {Matrix_left_f32_8x8::Splat(7.0f), Matrix_left_f32_8x8::Splat(42.0f)};
    Matrix_left_f32_8x8 v_12[2] = {Matrix_left_f32_8x8::Splat(-100.0f), Matrix_left_f32_8x8::Splat(-1.0f)};
    Matrix_left_f32_8x8 v_13[2][2] = {v_11, v_12};
    return v_13;
  }
  Matrix_left_f32_8x8 v_14 = Matrix_left_f32_8x8::Splat(0.0f);
  Matrix_left_f32_8x8 v_15[2] = {v_14, v_14};
  Matrix_left_f32_8x8 v_16[2][2] = {v_15, v_15};
  return v_16;
}

S make_struct() {
  if (non_uniform_condition) {
    S v_17 = {Matrix_left_f32_8x8::Splat(42.0f), Matrix_right_f32_8x8::Splat(100.0f)};
    return v_17;
  } else {
    S v_18 = {Matrix_left_f32_8x8::Splat(-7.0f), Matrix_right_f32_8x8::Splat(-42.0f)};
    return v_18;
  }
  S v_19 = {Matrix_left_f32_8x8::Splat(0.0f), Matrix_right_f32_8x8::Splat(0.0f)};
  return v_19;
}

S_Nested make_nested_struct() {
  if (non_uniform_condition) {
    S v_20 = {Matrix_left_f32_8x8::Splat(42.0f), Matrix_right_f32_8x8::Splat(100.0f)};
    S_Nested v_21 = {v_20};
    return v_21;
  } else {
    S v_22 = {Matrix_left_f32_8x8::Splat(-7.0f), Matrix_right_f32_8x8::Splat(-42.0f)};
    S_Nested v_23 = {v_22};
    return v_23;
  }
  S v_24 = {Matrix_left_f32_8x8::Splat(0.0f), Matrix_right_f32_8x8::Splat(0.0f)};
  S_Nested v_25 = {v_24};
  return v_25;
}

void main_inner(uint idx) {
  uint v_26 = 0u;
  buffer.GetDimensions(v_26);
  non_uniform_condition = (asfloat(buffer.Load((0u + (min(idx, ((v_26 / 4u) - 1u)) * 4u)))) == 0.0f);
  Matrix_left_f32_8x8 v_27 = make_matrix();
  uint v_28 = 0u;
  buffer.GetDimensions(v_28);
  if ((((0u + (64u * 7u)) + 8u) <= (v_28 / 4u))) {
    v_27.Store(buffer, 0u, 256u, MatrixLayout::RowMajor);
  }
  Matrix_left_f32_8x8 v_29[2] = make_array();
  Matrix_left_f32_8x8 v_30 = v_29[0u];
  uint v_31 = 0u;
  buffer.GetDimensions(v_31);
  if ((((0u + (64u * 7u)) + 8u) <= (v_31 / 4u))) {
    v_30.Store(buffer, 0u, 256u, MatrixLayout::RowMajor);
  }
  Matrix_left_f32_8x8 v_32[2][2] = make_nested_array();
  Matrix_left_f32_8x8 v_33 = v_32[1u][0u];
  uint v_34 = 0u;
  buffer.GetDimensions(v_34);
  if ((((0u + (64u * 7u)) + 8u) <= (v_34 / 4u))) {
    v_33.Store(buffer, 0u, 256u, MatrixLayout::RowMajor);
  }
  S v_35 = make_struct();
  Matrix_left_f32_8x8 v_36 = v_35.l;
  uint v_37 = 0u;
  buffer.GetDimensions(v_37);
  if ((((0u + (64u * 7u)) + 8u) <= (v_37 / 4u))) {
    v_36.Store(buffer, 0u, 256u, MatrixLayout::RowMajor);
  }
  S_Nested v_38 = make_nested_struct();
  Matrix_right_f32_8x8 v_39 = v_38.s.r;
  uint v_40 = 0u;
  buffer.GetDimensions(v_40);
  if ((((0u + (64u * 7u)) + 8u) <= (v_40 / 4u))) {
    v_39.Store(buffer, 0u, 256u, MatrixLayout::RowMajor);
  }
}

[numthreads(64, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.idx);
}

