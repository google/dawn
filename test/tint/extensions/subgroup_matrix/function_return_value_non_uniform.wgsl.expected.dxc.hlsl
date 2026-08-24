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
  uint v_29 = asuint(int(0));
  uint v_30 = asuint(int(64));
  uint v_31 = (v_29 + select((((v_30 != 0u) & (7u != 0u)) & (7u > (4294967295u / v_30))), 4294967295u, (v_30 * 7u)));
  uint v_32 = select((v_31 < v_29), 4294967295u, v_31);
  uint v_33 = (v_32 + 8u);
  bool v_34 = (select((v_33 < v_32), 4294967295u, v_33) <= (v_28 / 4u));
  v_27.Store(buffer, (0u + (select(v_34, v_29, 0u) * 4u)), (select(v_34, v_30, 8u) * 4u), MatrixLayout::RowMajor);
  Matrix_left_f32_8x8 v_35[2] = make_array();
  Matrix_left_f32_8x8 v_36 = v_35[0u];
  uint v_37 = 0u;
  buffer.GetDimensions(v_37);
  uint v_38 = asuint(int(0));
  uint v_39 = asuint(int(64));
  uint v_40 = (v_38 + select((((v_39 != 0u) & (7u != 0u)) & (7u > (4294967295u / v_39))), 4294967295u, (v_39 * 7u)));
  uint v_41 = select((v_40 < v_38), 4294967295u, v_40);
  uint v_42 = (v_41 + 8u);
  bool v_43 = (select((v_42 < v_41), 4294967295u, v_42) <= (v_37 / 4u));
  v_36.Store(buffer, (0u + (select(v_43, v_38, 0u) * 4u)), (select(v_43, v_39, 8u) * 4u), MatrixLayout::RowMajor);
  Matrix_left_f32_8x8 v_44[2][2] = make_nested_array();
  Matrix_left_f32_8x8 v_45 = v_44[1u][0u];
  uint v_46 = 0u;
  buffer.GetDimensions(v_46);
  uint v_47 = asuint(int(0));
  uint v_48 = asuint(int(64));
  uint v_49 = (v_47 + select((((v_48 != 0u) & (7u != 0u)) & (7u > (4294967295u / v_48))), 4294967295u, (v_48 * 7u)));
  uint v_50 = select((v_49 < v_47), 4294967295u, v_49);
  uint v_51 = (v_50 + 8u);
  bool v_52 = (select((v_51 < v_50), 4294967295u, v_51) <= (v_46 / 4u));
  v_45.Store(buffer, (0u + (select(v_52, v_47, 0u) * 4u)), (select(v_52, v_48, 8u) * 4u), MatrixLayout::RowMajor);
  S v_53 = make_struct();
  Matrix_left_f32_8x8 v_54 = v_53.l;
  uint v_55 = 0u;
  buffer.GetDimensions(v_55);
  uint v_56 = asuint(int(0));
  uint v_57 = asuint(int(64));
  uint v_58 = (v_56 + select((((v_57 != 0u) & (7u != 0u)) & (7u > (4294967295u / v_57))), 4294967295u, (v_57 * 7u)));
  uint v_59 = select((v_58 < v_56), 4294967295u, v_58);
  uint v_60 = (v_59 + 8u);
  bool v_61 = (select((v_60 < v_59), 4294967295u, v_60) <= (v_55 / 4u));
  v_54.Store(buffer, (0u + (select(v_61, v_56, 0u) * 4u)), (select(v_61, v_57, 8u) * 4u), MatrixLayout::RowMajor);
  S_Nested v_62 = make_nested_struct();
  Matrix_right_f32_8x8 v_63 = v_62.s.r;
  uint v_64 = 0u;
  buffer.GetDimensions(v_64);
  uint v_65 = asuint(int(0));
  uint v_66 = asuint(int(64));
  uint v_67 = (v_65 + select((((v_66 != 0u) & (7u != 0u)) & (7u > (4294967295u / v_66))), 4294967295u, (v_66 * 7u)));
  uint v_68 = select((v_67 < v_65), 4294967295u, v_67);
  uint v_69 = (v_68 + 8u);
  bool v_70 = (select((v_69 < v_68), 4294967295u, v_69) <= (v_64 / 4u));
  v_63.Store(buffer, (0u + (select(v_70, v_65, 0u) * 4u)), (select(v_70, v_66, 8u) * 4u), MatrixLayout::RowMajor);
}

[numthreads(64, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.idx);
}

