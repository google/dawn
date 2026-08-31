#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_result_f32_8x8 = Matrix<ComponentType::F32, 8, 8, MatrixUse::Accumulator, MatrixScope::Wave>;

ByteAddressBuffer in0 : register(t1);
ByteAddressBuffer in1 : register(t2);
ByteAddressBuffer in2 : register(t3);
ByteAddressBuffer in3 : register(t4);
ByteAddressBuffer in5 : register(t6);
ByteAddressBuffer in6 : register(t7);
RWByteAddressBuffer v : register(u0);
[numthreads(64, 1, 1)]
void main() {
  Matrix_result_f32_8x8 m0 = Matrix_result_f32_8x8::Load(in0, 0u, 64u, MatrixLayout::ColMajor);
  uint v_1 = 0u;
  v.GetDimensions(v_1);
  bool v_2 = (120u <= (v_1 / 4u));
  m0.Store(v, (0u + (select(v_2, 0u, 0u) * 4u)), (select(v_2, 16u, 8u) * 4u), MatrixLayout::ColMajor);
  Matrix_result_f32_8x8 m1 = Matrix_result_f32_8x8::Load(in1, 0u, 128u, MatrixLayout::ColMajor);
  uint v_3 = 0u;
  v.GetDimensions(v_3);
  bool v_4 = (120u <= (v_3 / 4u));
  m1.Store(v, (0u + (select(v_4, 0u, 0u) * 4u)), (select(v_4, 16u, 8u) * 4u), MatrixLayout::ColMajor);
  Matrix_result_f32_8x8 m2 = Matrix_result_f32_8x8::Load(in2, 0u, 256u, MatrixLayout::ColMajor);
  uint v_5 = 0u;
  v.GetDimensions(v_5);
  bool v_6 = (120u <= (v_5 / 4u));
  m2.Store(v, (0u + (select(v_6, 0u, 0u) * 4u)), (select(v_6, 16u, 8u) * 4u), MatrixLayout::ColMajor);
  Matrix_result_f32_8x8 m3 = Matrix_result_f32_8x8::Load(in3, 0u, 256u, MatrixLayout::ColMajor);
  uint v_7 = 0u;
  v.GetDimensions(v_7);
  bool v_8 = (120u <= (v_7 / 4u));
  m3.Store(v, (0u + (select(v_8, 0u, 0u) * 4u)), (select(v_8, 16u, 8u) * 4u), MatrixLayout::ColMajor);
  Matrix_result_f32_8x8 m5 = Matrix_result_f32_8x8::Load(in5, 0u, 64u, MatrixLayout::ColMajor);
  uint v_9 = 0u;
  v.GetDimensions(v_9);
  bool v_10 = (120u <= (v_9 / 4u));
  m5.Store(v, (0u + (select(v_10, 0u, 0u) * 4u)), (select(v_10, 16u, 8u) * 4u), MatrixLayout::ColMajor);
  Matrix_result_f32_8x8 m6 = Matrix_result_f32_8x8::Load(in6, 0u, 128u, MatrixLayout::ColMajor);
  uint v_11 = 0u;
  v.GetDimensions(v_11);
  bool v_12 = (120u <= (v_11 / 4u));
  m6.Store(v, (0u + (select(v_12, 0u, 0u) * 4u)), (select(v_12, 16u, 8u) * 4u), MatrixLayout::ColMajor);
}

