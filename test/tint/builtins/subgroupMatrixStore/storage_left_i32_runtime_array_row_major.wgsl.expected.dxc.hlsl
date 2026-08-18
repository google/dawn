#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_left_i32_8x8 = Matrix<ComponentType::I32, 8, 8, MatrixUse::A, MatrixScope::Wave>;

RWByteAddressBuffer out0 : register(u0);
RWByteAddressBuffer out1 : register(u1);
RWByteAddressBuffer out2 : register(u2);
RWByteAddressBuffer out3 : register(u3);
RWByteAddressBuffer out5 : register(u5);
RWByteAddressBuffer out6 : register(u6);
[numthreads(64, 1, 1)]
void main() {
  Matrix_left_i32_8x8 m = Matrix_left_i32_8x8::Splat(int(0));
  uint v = 0u;
  out0.GetDimensions(v);
  bool v_1 = (((0u + (16u * 7u)) + 8u) <= (v / 4u));
  m.Store(out0, (0u + (select(v_1, 0u, 0u) * 4u)), (select(v_1, 16u, 8u) * 4u), MatrixLayout::RowMajor);
  uint v_2 = 0u;
  out1.GetDimensions(v_2);
  bool v_3 = (((0u + (16u * 7u)) + 4u) <= (v_2 / 8u));
  m.Store(out1, (0u + (select(v_3, 0u, 0u) * 4u)), (select(v_3, 16u, 4u) * 4u), MatrixLayout::RowMajor);
  uint v_4 = 0u;
  out2.GetDimensions(v_4);
  bool v_5 = (((0u + (16u * 7u)) + 2u) <= (v_4 / 16u));
  m.Store(out2, (0u + (select(v_5, 0u, 0u) * 4u)), (select(v_5, 16u, 2u) * 4u), MatrixLayout::RowMajor);
  uint v_6 = 0u;
  out3.GetDimensions(v_6);
  bool v_7 = (((0u + (16u * 7u)) + 2u) <= (v_6 / 16u));
  m.Store(out3, (0u + (select(v_7, 0u, 0u) * 4u)), (select(v_7, 16u, 2u) * 4u), MatrixLayout::RowMajor);
  uint v_8 = 0u;
  out5.GetDimensions(v_8);
  bool v_9 = (((0u + (16u * 7u)) + 8u) <= (v_8 / 4u));
  m.Store(out5, (0u + (select(v_9, 0u, 0u) * 4u)), (select(v_9, 16u, 8u) * 4u), MatrixLayout::RowMajor);
  uint v_10 = 0u;
  out6.GetDimensions(v_10);
  bool v_11 = (((0u + (16u * 7u)) + 4u) <= (v_10 / 8u));
  m.Store(out6, (0u + (select(v_11, 0u, 0u) * 4u)), (select(v_11, 16u, 4u) * 4u), MatrixLayout::RowMajor);
}

