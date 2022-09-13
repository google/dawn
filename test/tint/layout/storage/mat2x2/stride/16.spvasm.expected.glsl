#version 310 es

struct strided_arr {
  vec2 el;
  uint pad;
  uint pad_1;
};

layout(binding = 0, std430) buffer SSBO_ssbo {
  strided_arr m[2];
} ssbo;

mat2 arr_to_mat2x2_stride_16(strided_arr arr[2]) {
  return mat2(arr[0u].el, arr[1u].el);
}

strided_arr[2] mat2x2_stride_16_to_arr(mat2 m) {
  strided_arr tint_symbol_1 = strided_arr(m[0u], 0u, 0u);
  strided_arr tint_symbol_2 = strided_arr(m[1u], 0u, 0u);
  strided_arr tint_symbol_3[2] = strided_arr[2](tint_symbol_1, tint_symbol_2);
  return tint_symbol_3;
}

void f_1() {
  mat2 x_15 = arr_to_mat2x2_stride_16(ssbo.m);
  strided_arr tint_symbol[2] = mat2x2_stride_16_to_arr(x_15);
  ssbo.m = tint_symbol;
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
