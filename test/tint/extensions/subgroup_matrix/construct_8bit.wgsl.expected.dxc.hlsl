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
  bool v_1 = (450u <= (v / 4u));
  Matrix_left_i8_8x8::Splat(int(0)).Store(ibuffer, (0u + (select(v_1, 0u, 0u) * 4u)), (select(v_1, 64u, 2u) * 4u), MatrixLayout::RowMajor);
  uint v_2 = 0u;
  ubuffer.GetDimensions(v_2);
  bool v_3 = (450u <= (v_2 / 4u));
  Matrix_right_u8_8x8::Splat(0u).Store(ubuffer, (0u + (select(v_3, 0u, 0u) * 4u)), (select(v_3, 64u, 2u) * 4u), MatrixLayout::RowMajor);
  uint v_4 = 0u;
  ibuffer.GetDimensions(v_4);
  bool v_5 = (450u <= (v_4 / 4u));
  Matrix_left_i8_8x8::Splat(int(-42)).Store(ibuffer, (0u + (select(v_5, 0u, 0u) * 4u)), (select(v_5, 64u, 2u) * 4u), MatrixLayout::RowMajor);
  uint v_6 = 0u;
  ubuffer.GetDimensions(v_6);
  bool v_7 = (450u <= (v_6 / 4u));
  Matrix_right_u8_8x8::Splat(42u).Store(ubuffer, (0u + (select(v_7, 0u, 0u) * 4u)), (select(v_7, 64u, 2u) * 4u), MatrixLayout::RowMajor);
}

