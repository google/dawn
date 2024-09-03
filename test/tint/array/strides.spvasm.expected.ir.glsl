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

S s;
void tint_store_and_preserve_padding_4(inout strided_arr target, strided_arr value_param) {
  target.el = value_param.el;
}
void tint_store_and_preserve_padding_3(inout strided_arr target[2], strided_arr value_param[2]) {
  {
    uint v = 0u;
    v = 0u;
    while(true) {
      uint v_1 = v;
      if ((v_1 >= 2u)) {
        break;
      }
      tint_store_and_preserve_padding_4(target[v_1], value_param[v_1]);
      {
        v = (v_1 + 1u);
      }
      continue;
    }
  }
}
void tint_store_and_preserve_padding_2(inout strided_arr target[3][2], strided_arr value_param[3][2]) {
  {
    uint v_2 = 0u;
    v_2 = 0u;
    while(true) {
      uint v_3 = v_2;
      if ((v_3 >= 3u)) {
        break;
      }
      tint_store_and_preserve_padding_3(target[v_3], value_param[v_3]);
      {
        v_2 = (v_3 + 1u);
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
    uint v_4 = 0u;
    v_4 = 0u;
    while(true) {
      uint v_5 = v_4;
      if ((v_5 >= 4u)) {
        break;
      }
      tint_store_and_preserve_padding_1(target[v_5], value_param[v_5]);
      {
        v_4 = (v_5 + 1u);
      }
      continue;
    }
  }
}
void f_1() {
  strided_arr_1 x_19[4] = s.a;
  strided_arr x_24[3][2] = s.a[3].el;
  strided_arr x_28[2] = s.a[3].el[2];
  float x_32 = s.a[3].el[2][1].el;
  tint_store_and_preserve_padding(s.a, strided_arr_1[4](strided_arr_1(strided_arr[3][2](strided_arr[2](strided_arr(0.0f), strided_arr(0.0f)), strided_arr[2](strided_arr(0.0f), strided_arr(0.0f)), strided_arr[2](strided_arr(0.0f), strided_arr(0.0f)))), strided_arr_1(strided_arr[3][2](strided_arr[2](strided_arr(0.0f), strided_arr(0.0f)), strided_arr[2](strided_arr(0.0f), strided_arr(0.0f)), strided_arr[2](strided_arr(0.0f), strided_arr(0.0f)))), strided_arr_1(strided_arr[3][2](strided_arr[2](strided_arr(0.0f), strided_arr(0.0f)), strided_arr[2](strided_arr(0.0f), strided_arr(0.0f)), strided_arr[2](strided_arr(0.0f), strided_arr(0.0f)))), strided_arr_1(strided_arr[3][2](strided_arr[2](strided_arr(0.0f), strided_arr(0.0f)), strided_arr[2](strided_arr(0.0f), strided_arr(0.0f)), strided_arr[2](strided_arr(0.0f), strided_arr(0.0f))))));
  s.a[3].el[2][1].el = 5.0f;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f_1();
}
