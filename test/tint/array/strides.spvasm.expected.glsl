#version 310 es

struct strided_arr {
  float el;
  uint pad;
};

struct strided_arr_1 {
  strided_arr el[3][2];
  uint pad_1;
  uint pad_2;
  uint pad_3;
  uint pad_4;
  uint pad_5;
  uint pad_6;
  uint pad_7;
  uint pad_8;
  uint pad_9;
  uint pad_10;
  uint pad_11;
  uint pad_12;
  uint pad_13;
  uint pad_14;
  uint pad_15;
  uint pad_16;
  uint pad_17;
  uint pad_18;
  uint pad_19;
  uint pad_20;
};

layout(binding = 0, std430) buffer S_ssbo {
  strided_arr_1 a[4];
} s;

void f_1() {
  strided_arr_1 x_19[4] = s.a;
  strided_arr x_24[3][2] = s.a[3].el;
  strided_arr x_28[2] = s.a[3].el[2];
  float x_32 = s.a[3].el[2][1].el;
  strided_arr_1 tint_symbol[4] = strided_arr_1[4](strided_arr_1(strided_arr[3][2](strided_arr[2](strided_arr(0.0f, 0u), strided_arr(0.0f, 0u)), strided_arr[2](strided_arr(0.0f, 0u), strided_arr(0.0f, 0u)), strided_arr[2](strided_arr(0.0f, 0u), strided_arr(0.0f, 0u))), 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u), strided_arr_1(strided_arr[3][2](strided_arr[2](strided_arr(0.0f, 0u), strided_arr(0.0f, 0u)), strided_arr[2](strided_arr(0.0f, 0u), strided_arr(0.0f, 0u)), strided_arr[2](strided_arr(0.0f, 0u), strided_arr(0.0f, 0u))), 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u), strided_arr_1(strided_arr[3][2](strided_arr[2](strided_arr(0.0f, 0u), strided_arr(0.0f, 0u)), strided_arr[2](strided_arr(0.0f, 0u), strided_arr(0.0f, 0u)), strided_arr[2](strided_arr(0.0f, 0u), strided_arr(0.0f, 0u))), 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u), strided_arr_1(strided_arr[3][2](strided_arr[2](strided_arr(0.0f, 0u), strided_arr(0.0f, 0u)), strided_arr[2](strided_arr(0.0f, 0u), strided_arr(0.0f, 0u)), strided_arr[2](strided_arr(0.0f, 0u), strided_arr(0.0f, 0u))), 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u));
  s.a = tint_symbol;
  s.a[3].el[2][1].el = 5.0f;
  return;
}

void f() {
  f_1();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
