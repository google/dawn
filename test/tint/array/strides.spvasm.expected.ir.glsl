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

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  S tint_symbol;
} v;
void tint_store_and_preserve_padding_4(inout strided_arr target, strided_arr value_param) {
  target.el = value_param.el;
}
void tint_store_and_preserve_padding_3(inout strided_arr target[2], strided_arr value_param[2]) {
  {
    uint v_1 = 0u;
    v_1 = 0u;
    while(true) {
      uint v_2 = v_1;
      if ((v_2 >= 2u)) {
        break;
      }
      tint_store_and_preserve_padding_4(target[v_2], value_param[v_2]);
      {
        v_1 = (v_2 + 1u);
      }
      continue;
    }
  }
}
void tint_store_and_preserve_padding_2(inout strided_arr target[3][2], strided_arr value_param[3][2]) {
  {
    uint v_3 = 0u;
    v_3 = 0u;
    while(true) {
      uint v_4 = v_3;
      if ((v_4 >= 3u)) {
        break;
      }
      tint_store_and_preserve_padding_3(target[v_4], value_param[v_4]);
      {
        v_3 = (v_4 + 1u);
      }
      continue;
    }
  }
}
void tint_store_and_preserve_padding_1(inout strided_arr_1 target, strided_arr_1 value_param) {
  tint_store_and_preserve_padding_2(target.el, value_param.el);
}
void tint_store_and_preserve_padding(inout strided_arr_1 target[4], strided_arr_1 value_param[4]) {
  {
    uint v_5 = 0u;
    v_5 = 0u;
    while(true) {
      uint v_6 = v_5;
      if ((v_6 >= 4u)) {
        break;
      }
      tint_store_and_preserve_padding_1(target[v_6], value_param[v_6]);
      {
        v_5 = (v_6 + 1u);
      }
      continue;
    }
  }
}
void f_1() {
  strided_arr_1 x_19[4] = v.tint_symbol.a;
  strided_arr x_24[3][2] = v.tint_symbol.a[3].el;
  strided_arr x_28[2] = v.tint_symbol.a[3].el[2];
  float x_32 = v.tint_symbol.a[3].el[2][1].el;
  tint_store_and_preserve_padding(v.tint_symbol.a, strided_arr_1[4](strided_arr_1(strided_arr[3][2](strided_arr[2](strided_arr(0.0f), strided_arr(0.0f)), strided_arr[2](strided_arr(0.0f), strided_arr(0.0f)), strided_arr[2](strided_arr(0.0f), strided_arr(0.0f)))), strided_arr_1(strided_arr[3][2](strided_arr[2](strided_arr(0.0f), strided_arr(0.0f)), strided_arr[2](strided_arr(0.0f), strided_arr(0.0f)), strided_arr[2](strided_arr(0.0f), strided_arr(0.0f)))), strided_arr_1(strided_arr[3][2](strided_arr[2](strided_arr(0.0f), strided_arr(0.0f)), strided_arr[2](strided_arr(0.0f), strided_arr(0.0f)), strided_arr[2](strided_arr(0.0f), strided_arr(0.0f)))), strided_arr_1(strided_arr[3][2](strided_arr[2](strided_arr(0.0f), strided_arr(0.0f)), strided_arr[2](strided_arr(0.0f), strided_arr(0.0f)), strided_arr[2](strided_arr(0.0f), strided_arr(0.0f))))));
  v.tint_symbol.a[3].el[2][1].el = 5.0f;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f_1();
}
