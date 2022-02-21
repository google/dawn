#version 310 es

struct strided_arr {
  float el;
};

struct strided_arr_1 {
  strided_arr el[3][2];
};

struct S {
  strided_arr_1 a[4];
};

layout(binding = 0, std430) buffer S_1 {
  strided_arr_1 a[4];
} s;
void f_1() {
  strided_arr_1 x_19[4] = s.a;
  strided_arr x_24[3][2] = s.a[3].el;
  strided_arr x_28[2] = s.a[3].el[2];
  float x_32 = s.a[3].el[2][1].el;
  strided_arr_1 tint_symbol[4] = strided_arr_1[4](strided_arr_1(strided_arr[3][2](strided_arr[2](strided_arr(0.0f), strided_arr(0.0f)), strided_arr[2](strided_arr(0.0f), strided_arr(0.0f)), strided_arr[2](strided_arr(0.0f), strided_arr(0.0f)))), strided_arr_1(strided_arr[3][2](strided_arr[2](strided_arr(0.0f), strided_arr(0.0f)), strided_arr[2](strided_arr(0.0f), strided_arr(0.0f)), strided_arr[2](strided_arr(0.0f), strided_arr(0.0f)))), strided_arr_1(strided_arr[3][2](strided_arr[2](strided_arr(0.0f), strided_arr(0.0f)), strided_arr[2](strided_arr(0.0f), strided_arr(0.0f)), strided_arr[2](strided_arr(0.0f), strided_arr(0.0f)))), strided_arr_1(strided_arr[3][2](strided_arr[2](strided_arr(0.0f), strided_arr(0.0f)), strided_arr[2](strided_arr(0.0f), strided_arr(0.0f)), strided_arr[2](strided_arr(0.0f), strided_arr(0.0f)))));
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
