#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_right_i32_8x8 = Matrix<ComponentType::I32, 8, 8, MatrixUse::B, MatrixScope::Wave>;

ByteAddressBuffer in0 : register(t1);
ByteAddressBuffer in1 : register(t2);
ByteAddressBuffer in2 : register(t3);
ByteAddressBuffer in3 : register(t4);
ByteAddressBuffer in5 : register(t6);
ByteAddressBuffer in6 : register(t7);
RWByteAddressBuffer v : register(u0);
[numthreads(64, 1, 1)]
void main() {
  uint v_1 = 0u;
  in0.GetDimensions(v_1);
  bool v_2 = (120u <= (v_1 / 4u));
  Matrix_right_i32_8x8 m0 = Matrix_right_i32_8x8::Load(in0, (0u + (select(v_2, 0u, 0u) * 4u)), (select(v_2, 16u, 8u) * 4u), MatrixLayout::RowMajor);
  uint v_3 = 0u;
  v.GetDimensions(v_3);
  bool v_4 = (120u <= (v_3 / 4u));
  m0.Store(v, (0u + (select(v_4, 0u, 0u) * 4u)), (select(v_4, 16u, 8u) * 4u), MatrixLayout::ColMajor);
  uint v_5 = 0u;
  in1.GetDimensions(v_5);
  bool v_6 = (116u <= (v_5 / 8u));
  Matrix_right_i32_8x8 m1 = Matrix_right_i32_8x8::Load(in1, (0u + (select(v_6, 0u, 0u) * 8u)), (select(v_6, 16u, 4u) * 8u), MatrixLayout::RowMajor);
  uint v_7 = 0u;
  v.GetDimensions(v_7);
  bool v_8 = (120u <= (v_7 / 4u));
  m1.Store(v, (0u + (select(v_8, 0u, 0u) * 4u)), (select(v_8, 16u, 8u) * 4u), MatrixLayout::ColMajor);
  uint v_9 = 0u;
  in2.GetDimensions(v_9);
  bool v_10 = (114u <= (v_9 / 16u));
  Matrix_right_i32_8x8 m2 = Matrix_right_i32_8x8::Load(in2, (0u + (select(v_10, 0u, 0u) * 16u)), (select(v_10, 16u, 2u) * 16u), MatrixLayout::RowMajor);
  uint v_11 = 0u;
  v.GetDimensions(v_11);
  bool v_12 = (120u <= (v_11 / 4u));
  m2.Store(v, (0u + (select(v_12, 0u, 0u) * 4u)), (select(v_12, 16u, 8u) * 4u), MatrixLayout::ColMajor);
  uint v_13 = 0u;
  in3.GetDimensions(v_13);
  bool v_14 = (114u <= (v_13 / 16u));
  Matrix_right_i32_8x8 m3 = Matrix_right_i32_8x8::Load(in3, (0u + (select(v_14, 0u, 0u) * 16u)), (select(v_14, 16u, 2u) * 16u), MatrixLayout::RowMajor);
  uint v_15 = 0u;
  v.GetDimensions(v_15);
  bool v_16 = (120u <= (v_15 / 4u));
  m3.Store(v, (0u + (select(v_16, 0u, 0u) * 4u)), (select(v_16, 16u, 8u) * 4u), MatrixLayout::ColMajor);
  uint v_17 = 0u;
  in5.GetDimensions(v_17);
  bool v_18 = (120u <= (v_17 / 4u));
  Matrix_right_i32_8x8 m5 = Matrix_right_i32_8x8::Load(in5, (0u + (select(v_18, 0u, 0u) * 4u)), (select(v_18, 16u, 8u) * 4u), MatrixLayout::RowMajor);
  uint v_19 = 0u;
  v.GetDimensions(v_19);
  bool v_20 = (120u <= (v_19 / 4u));
  m5.Store(v, (0u + (select(v_20, 0u, 0u) * 4u)), (select(v_20, 16u, 8u) * 4u), MatrixLayout::ColMajor);
  uint v_21 = 0u;
  in6.GetDimensions(v_21);
  bool v_22 = (116u <= (v_21 / 8u));
  Matrix_right_i32_8x8 m6 = Matrix_right_i32_8x8::Load(in6, (0u + (select(v_22, 0u, 0u) * 8u)), (select(v_22, 16u, 4u) * 8u), MatrixLayout::RowMajor);
  uint v_23 = 0u;
  v.GetDimensions(v_23);
  bool v_24 = (120u <= (v_23 / 4u));
  m6.Store(v, (0u + (select(v_24, 0u, 0u) * 4u)), (select(v_24, 16u, 8u) * 4u), MatrixLayout::ColMajor);
}

