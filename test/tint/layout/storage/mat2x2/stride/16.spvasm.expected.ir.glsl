#version 310 es

struct strided_arr {
  vec2 el;
};

struct SSBO {
  strided_arr m[2];
};

SSBO ssbo;
strided_arr[2] mat2x2_stride_16_to_arr(mat2 m) {
  strided_arr v = strided_arr(m[0u]);
  return strided_arr[2](v, strided_arr(m[1u]));
}
mat2 arr_to_mat2x2_stride_16(strided_arr arr[2]) {
  return mat2(arr[0u].el, arr[1u].el);
}
void tint_store_and_preserve_padding_1(inout strided_arr target, strided_arr value_param) {
  target.el = value_param.el;
}
void tint_store_and_preserve_padding(inout strided_arr target[2], strided_arr value_param[2]) {
  {
    uint v_1 = 0u;
    v_1 = 0u;
    while(true) {
      uint v_2 = v_1;
      if ((v_2 >= 2u)) {
        break;
      }
      tint_store_and_preserve_padding_1(target[v_2], value_param[v_2]);
      {
        v_1 = (v_2 + 1u);
      }
      continue;
    }
  }
}
void f_1() {
  tint_store_and_preserve_padding(ssbo.m, mat2x2_stride_16_to_arr(arr_to_mat2x2_stride_16(ssbo.m)));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f_1();
}
