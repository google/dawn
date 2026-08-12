#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_result_i8_8x8 = Matrix<ComponentType::I8, 8, 8, MatrixUse::Accumulator, MatrixScope::Wave>;

ByteAddressBuffer in0 : register(t1);
ByteAddressBuffer in1 : register(t2);
ByteAddressBuffer in2 : register(t3);
ByteAddressBuffer in3 : register(t4);
ByteAddressBuffer in4 : register(t5);
ByteAddressBuffer in5 : register(t6);
ByteAddressBuffer in6 : register(t7);
RWByteAddressBuffer v : register(u0);
[numthreads(64, 1, 1)]
void main() {
  Matrix_result_i8_8x8 m0 = Matrix_result_i8_8x8::Load(in0, 0u, 64u, MatrixLayout::RowMajor);
  uint v_1 = 0u;
  v.GetDimensions(v_1);
  uint v_2 = asuint(int(0));
  uint v_3 = asuint(int(16));
  bool v_4 = ((((v_2 + (v_3 * 7u)) * 4u) + 8u) <= ((v_1 / 4u) * 4u));
  m0.Store(v, (0u + (select(v_4, v_2, 0u) * 4u)), (select(v_4, v_3, 2u) * 4u), MatrixLayout::ColMajor);
  Matrix_result_i8_8x8 m1 = Matrix_result_i8_8x8::Load(in1, 0u, 64u, MatrixLayout::RowMajor);
  uint v_5 = 0u;
  v.GetDimensions(v_5);
  uint v_6 = asuint(int(0));
  uint v_7 = asuint(int(16));
  bool v_8 = ((((v_6 + (v_7 * 7u)) * 4u) + 8u) <= ((v_5 / 4u) * 4u));
  m1.Store(v, (0u + (select(v_8, v_6, 0u) * 4u)), (select(v_8, v_7, 2u) * 4u), MatrixLayout::ColMajor);
  Matrix_result_i8_8x8 m2 = Matrix_result_i8_8x8::Load(in2, 0u, 64u, MatrixLayout::RowMajor);
  uint v_9 = 0u;
  v.GetDimensions(v_9);
  uint v_10 = asuint(int(0));
  uint v_11 = asuint(int(16));
  bool v_12 = ((((v_10 + (v_11 * 7u)) * 4u) + 8u) <= ((v_9 / 4u) * 4u));
  m2.Store(v, (0u + (select(v_12, v_10, 0u) * 4u)), (select(v_12, v_11, 2u) * 4u), MatrixLayout::ColMajor);
  Matrix_result_i8_8x8 m3 = Matrix_result_i8_8x8::Load(in3, 0u, 64u, MatrixLayout::RowMajor);
  uint v_13 = 0u;
  v.GetDimensions(v_13);
  uint v_14 = asuint(int(0));
  uint v_15 = asuint(int(16));
  bool v_16 = ((((v_14 + (v_15 * 7u)) * 4u) + 8u) <= ((v_13 / 4u) * 4u));
  m3.Store(v, (0u + (select(v_16, v_14, 0u) * 4u)), (select(v_16, v_15, 2u) * 4u), MatrixLayout::ColMajor);
  Matrix_result_i8_8x8 m4 = Matrix_result_i8_8x8::Load(in4, 0u, 64u, MatrixLayout::RowMajor);
  uint v_17 = 0u;
  v.GetDimensions(v_17);
  uint v_18 = asuint(int(0));
  uint v_19 = asuint(int(16));
  bool v_20 = ((((v_18 + (v_19 * 7u)) * 4u) + 8u) <= ((v_17 / 4u) * 4u));
  m4.Store(v, (0u + (select(v_20, v_18, 0u) * 4u)), (select(v_20, v_19, 2u) * 4u), MatrixLayout::ColMajor);
  Matrix_result_i8_8x8 m5 = Matrix_result_i8_8x8::Load(in5, 0u, 64u, MatrixLayout::RowMajor);
  uint v_21 = 0u;
  v.GetDimensions(v_21);
  uint v_22 = asuint(int(0));
  uint v_23 = asuint(int(16));
  bool v_24 = ((((v_22 + (v_23 * 7u)) * 4u) + 8u) <= ((v_21 / 4u) * 4u));
  m5.Store(v, (0u + (select(v_24, v_22, 0u) * 4u)), (select(v_24, v_23, 2u) * 4u), MatrixLayout::ColMajor);
  Matrix_result_i8_8x8 m6 = Matrix_result_i8_8x8::Load(in6, 0u, 64u, MatrixLayout::RowMajor);
  uint v_25 = 0u;
  v.GetDimensions(v_25);
  uint v_26 = asuint(int(0));
  uint v_27 = asuint(int(16));
  bool v_28 = ((((v_26 + (v_27 * 7u)) * 4u) + 8u) <= ((v_25 / 4u) * 4u));
  m6.Store(v, (0u + (select(v_28, v_26, 0u) * 4u)), (select(v_28, v_27, 2u) * 4u), MatrixLayout::ColMajor);
}

