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

struct S {
  strided_arr_1 a[4];
};

layout(binding = 0, std430) buffer s_block_ssbo {
  S inner;
} s;

void assign_and_preserve_padding_4_s_a_X_el_X_X(uint dest[3], strided_arr value) {
  s.inner.a[dest[0]].el[dest[0]][dest[0]].el = value.el;
}

void assign_and_preserve_padding_3_s_a_X_el_X(uint dest[2], strided_arr value[2]) {
  {
    for(uint i = 0u; (i < 2u); i = (i + 1u)) {
      uint tint_symbol[3] = uint[3](dest[0u], dest[1u], i);
      assign_and_preserve_padding_4_s_a_X_el_X_X(tint_symbol, value[i]);
    }
  }
}

void assign_and_preserve_padding_2_s_a_X_el(uint dest[1], strided_arr value[3][2]) {
  {
    for(uint i = 0u; (i < 3u); i = (i + 1u)) {
      uint tint_symbol_1[2] = uint[2](dest[0u], i);
      assign_and_preserve_padding_3_s_a_X_el_X(tint_symbol_1, value[i]);
    }
  }
}

void assign_and_preserve_padding_1_s_a_X(uint dest[1], strided_arr_1 value) {
  uint tint_symbol_2[1] = uint[1](dest[0u]);
  assign_and_preserve_padding_2_s_a_X_el(tint_symbol_2, value.el);
}

void assign_and_preserve_padding_s_a(strided_arr_1 value[4]) {
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      uint tint_symbol_3[1] = uint[1](i);
      assign_and_preserve_padding_1_s_a_X(tint_symbol_3, value[i]);
    }
  }
}

void f_1() {
  strided_arr_1 x_19[4] = s.inner.a;
  strided_arr x_24[3][2] = s.inner.a[3].el;
  strided_arr x_28[2] = s.inner.a[3].el[2];
  float x_32 = s.inner.a[3].el[2][1].el;
  strided_arr_1 tint_symbol_4[4] = strided_arr_1[4](strided_arr_1(strided_arr[3][2](strided_arr[2](strided_arr(0.0f, 0u), strided_arr(0.0f, 0u)), strided_arr[2](strided_arr(0.0f, 0u), strided_arr(0.0f, 0u)), strided_arr[2](strided_arr(0.0f, 0u), strided_arr(0.0f, 0u))), 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u), strided_arr_1(strided_arr[3][2](strided_arr[2](strided_arr(0.0f, 0u), strided_arr(0.0f, 0u)), strided_arr[2](strided_arr(0.0f, 0u), strided_arr(0.0f, 0u)), strided_arr[2](strided_arr(0.0f, 0u), strided_arr(0.0f, 0u))), 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u), strided_arr_1(strided_arr[3][2](strided_arr[2](strided_arr(0.0f, 0u), strided_arr(0.0f, 0u)), strided_arr[2](strided_arr(0.0f, 0u), strided_arr(0.0f, 0u)), strided_arr[2](strided_arr(0.0f, 0u), strided_arr(0.0f, 0u))), 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u), strided_arr_1(strided_arr[3][2](strided_arr[2](strided_arr(0.0f, 0u), strided_arr(0.0f, 0u)), strided_arr[2](strided_arr(0.0f, 0u), strided_arr(0.0f, 0u)), strided_arr[2](strided_arr(0.0f, 0u), strided_arr(0.0f, 0u))), 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u));
  assign_and_preserve_padding_s_a(tint_symbol_4);
  s.inner.a[3].el[2][1].el = 5.0f;
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
