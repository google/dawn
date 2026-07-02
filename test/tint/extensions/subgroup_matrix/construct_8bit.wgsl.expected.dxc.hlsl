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
  if ((((asuint(int(0)) + (asuint(int(64)) * 7u)) + 8u) <= ((v / 4u) * 4u))) {
    Matrix_left_i8_8x8::Splat(int(0)).Store(ibuffer, 0u, 64u, MatrixLayout::RowMajor);
  }
  uint v_1 = 0u;
  ubuffer.GetDimensions(v_1);
  if ((((asuint(int(0)) + (asuint(int(64)) * 7u)) + 8u) <= ((v_1 / 4u) * 4u))) {
    Matrix_right_u8_8x8::Splat(0u).Store(ubuffer, 0u, 64u, MatrixLayout::RowMajor);
  }
  uint v_2 = 0u;
  ibuffer.GetDimensions(v_2);
  if ((((asuint(int(0)) + (asuint(int(64)) * 7u)) + 8u) <= ((v_2 / 4u) * 4u))) {
    Matrix_left_i8_8x8::Splat(int(-42)).Store(ibuffer, 0u, 64u, MatrixLayout::RowMajor);
  }
  uint v_3 = 0u;
  ubuffer.GetDimensions(v_3);
  if ((((asuint(int(0)) + (asuint(int(64)) * 7u)) + 8u) <= ((v_3 / 4u) * 4u))) {
    Matrix_right_u8_8x8::Splat(42u).Store(ubuffer, 0u, 64u, MatrixLayout::RowMajor);
  }
}

