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
  uint v = 0u;
  buffer.GetDimensions(v);
  uint v_1 = asuint(int(0));
  uint v_2 = asuint(int(64));
  uint v_3 = (v_1 + select((((v_2 != 0u) & (7u != 0u)) & (7u > (4294967295u / v_2))), 4294967295u, (v_2 * 7u)));
  uint v_4 = select((v_3 < v_1), 4294967295u, v_3);
  uint v_5 = (v_4 + 8u);
  bool v_6 = (select((v_5 < v_4), 4294967295u, v_5) <= (v / 4u));
  m.Store(buffer, (0u + (select(v_6, v_1, 0u) * 4u)), (select(v_6, v_2, 8u) * 4u), MatrixLayout::RowMajor);
  uint v_7 = 0u;
  buffer.GetDimensions(v_7);
  uint v_8 = asuint(int(0));
  uint v_9 = asuint(int(64));
  uint v_10 = (v_8 + select((((v_9 != 0u) & (7u != 0u)) & (7u > (4294967295u / v_9))), 4294967295u, (v_9 * 7u)));
  uint v_11 = select((v_10 < v_8), 4294967295u, v_10);
  uint v_12 = (v_11 + 8u);
  bool v_13 = (select((v_12 < v_11), 4294967295u, v_12) <= (v_7 / 4u));
  m_array[0u].Store(buffer, (0u + (select(v_13, v_8, 0u) * 4u)), (select(v_13, v_9, 8u) * 4u), MatrixLayout::RowMajor);
  uint v_14 = 0u;
  buffer.GetDimensions(v_14);
  uint v_15 = asuint(int(0));
  uint v_16 = asuint(int(64));
  uint v_17 = (v_15 + select((((v_16 != 0u) & (7u != 0u)) & (7u > (4294967295u / v_16))), 4294967295u, (v_16 * 7u)));
  uint v_18 = select((v_17 < v_15), 4294967295u, v_17);
  uint v_19 = (v_18 + 8u);
  bool v_20 = (select((v_19 < v_18), 4294967295u, v_19) <= (v_14 / 4u));
  m_nested_array[1u][2u].Store(buffer, (0u + (select(v_20, v_15, 0u) * 4u)), (select(v_20, v_16, 8u) * 4u), MatrixLayout::RowMajor);
  uint v_21 = 0u;
  buffer.GetDimensions(v_21);
  uint v_22 = asuint(int(0));
  uint v_23 = asuint(int(64));
  uint v_24 = (v_22 + select((((v_23 != 0u) & (7u != 0u)) & (7u > (4294967295u / v_23))), 4294967295u, (v_23 * 7u)));
  uint v_25 = select((v_24 < v_22), 4294967295u, v_24);
  uint v_26 = (v_25 + 8u);
  bool v_27 = (select((v_26 < v_25), 4294967295u, v_26) <= (v_21 / 4u));
  m_struct.l.Store(buffer, (0u + (select(v_27, v_22, 0u) * 4u)), (select(v_27, v_23, 8u) * 4u), MatrixLayout::RowMajor);
  uint v_28 = 0u;
  buffer.GetDimensions(v_28);
  uint v_29 = asuint(int(0));
  uint v_30 = asuint(int(64));
  uint v_31 = (v_29 + select((((v_30 != 0u) & (7u != 0u)) & (7u > (4294967295u / v_30))), 4294967295u, (v_30 * 7u)));
  uint v_32 = select((v_31 < v_29), 4294967295u, v_31);
  uint v_33 = (v_32 + 8u);
  bool v_34 = (select((v_33 < v_32), 4294967295u, v_33) <= (v_28 / 4u));
  m_nested_struct.s.r.Store(buffer, (0u + (select(v_34, v_29, 0u) * 4u)), (select(v_34, v_30, 8u) * 4u), MatrixLayout::RowMajor);
}

[numthreads(64, 1, 1)]
void main() {
  Matrix_left_f32_8x8 m = Matrix_left_f32_8x8::Splat(0.0f);
  Matrix_left_f32_8x8 v_35 = Matrix_left_f32_8x8::Splat(0.0f);
  Matrix_left_f32_8x8 m_array[4] = {v_35, v_35, v_35, v_35};
  Matrix_left_f32_8x8 v_36 = Matrix_left_f32_8x8::Splat(0.0f);
  Matrix_left_f32_8x8 v_37[4] = {v_36, v_36, v_36, v_36};
  Matrix_left_f32_8x8 m_nested_array[4][4] = {v_37, v_37, v_37, v_37};
  S m_struct = {Matrix_left_f32_8x8::Splat(0.0f), Matrix_right_f32_8x8::Splat(0.0f)};
  S v_38 = {Matrix_left_f32_8x8::Splat(0.0f), Matrix_right_f32_8x8::Splat(0.0f)};
  S_Nested m_nested_struct = {v_38};
  foo(m, m_array, m_nested_array, m_struct, m_nested_struct);
}

