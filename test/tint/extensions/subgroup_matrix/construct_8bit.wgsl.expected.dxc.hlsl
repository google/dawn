#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_left_i8_8x8 = Matrix<ComponentType::I8, 8, 8, MatrixUse::A, MatrixScope::Wave>;
using Matrix_right_u8_8x8 = Matrix<ComponentType::U8, 8, 8, MatrixUse::B, MatrixScope::Wave>;

RWByteAddressBuffer ibuffer : register(u0);
RWByteAddressBuffer ubuffer : register(u1);
[numthreads(64, 1, 1)]
void main() {
  uint v = 0u;
  ibuffer.GetDimensions(v);
  uint v_1 = asuint(int(0));
  uint v_2 = asuint(int(64));
  bool v_3 = (((v_1 + (v_2 * 7u)) + 2u) <= (v / 4u));
  Matrix_left_i8_8x8::Splat(int(0)).Store(ibuffer, (0u + (select(v_3, v_1, 0u) * 4u)), (select(v_3, v_2, 2u) * 4u), MatrixLayout::RowMajor);
  uint v_4 = 0u;
  ubuffer.GetDimensions(v_4);
  uint v_5 = asuint(int(0));
  uint v_6 = asuint(int(64));
  bool v_7 = (((v_5 + (v_6 * 7u)) + 2u) <= (v_4 / 4u));
  Matrix_right_u8_8x8::Splat(0u).Store(ubuffer, (0u + (select(v_7, v_5, 0u) * 4u)), (select(v_7, v_6, 2u) * 4u), MatrixLayout::RowMajor);
  uint v_8 = 0u;
  ibuffer.GetDimensions(v_8);
  uint v_9 = asuint(int(0));
  uint v_10 = asuint(int(64));
  bool v_11 = (((v_9 + (v_10 * 7u)) + 2u) <= (v_8 / 4u));
  Matrix_left_i8_8x8::Splat(int(-42)).Store(ibuffer, (0u + (select(v_11, v_9, 0u) * 4u)), (select(v_11, v_10, 2u) * 4u), MatrixLayout::RowMajor);
  uint v_12 = 0u;
  ubuffer.GetDimensions(v_12);
  uint v_13 = asuint(int(0));
  uint v_14 = asuint(int(64));
  bool v_15 = (((v_13 + (v_14 * 7u)) + 2u) <= (v_12 / 4u));
  Matrix_right_u8_8x8::Splat(42u).Store(ubuffer, (0u + (select(v_15, v_13, 0u) * 4u)), (select(v_15, v_14, 2u) * 4u), MatrixLayout::RowMajor);
}

