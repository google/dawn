#version 310 es
precision mediump float;

struct tint_padded_array_element {
  vec2 el;
};

layout (binding = 0) buffer SSBO_1 {
  tint_padded_array_element m[2];
} ssbo;

mat2 arr_to_mat2x2_stride_16(tint_padded_array_element arr[2]) {
  return mat2(arr[0u].el, arr[1u].el);
}

tint_padded_array_element[2] mat2x2_stride_16_to_arr(mat2 mat) {
  tint_padded_array_element tint_symbol[2] = tint_padded_array_element[2](tint_padded_array_element(mat[0u]), tint_padded_array_element(mat[1u]));
  return tint_symbol;
}

void f_1() {
  mat2 x_15 = arr_to_mat2x2_stride_16(ssbo.m);
  ssbo.m = mat2x2_stride_16_to_arr(x_15);
  return;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void f() {
  f_1();
  return;
}
void main() {
  f();
}


