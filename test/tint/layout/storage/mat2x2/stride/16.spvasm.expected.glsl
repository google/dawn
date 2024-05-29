#version 310 es

struct strided_arr {
  vec2 el;
  uint pad;
  uint pad_1;
};

struct SSBO {
  strided_arr m[2];
};

layout(binding = 0, std430) buffer ssbo_block_ssbo {
  SSBO inner;
} ssbo;

strided_arr[2] mat2x2_stride_16_to_arr(mat2 m) {
  strided_arr tint_symbol_2 = strided_arr(m[0u], 0u, 0u);
  strided_arr tint_symbol_3 = strided_arr(m[1u], 0u, 0u);
  strided_arr tint_symbol_4[2] = strided_arr[2](tint_symbol_2, tint_symbol_3);
  return tint_symbol_4;
}

mat2 arr_to_mat2x2_stride_16(strided_arr arr[2]) {
  return mat2(arr[0u].el, arr[1u].el);
}

void assign_and_preserve_padding_1_ssbo_inner_m_X(uint dest[1], strided_arr value) {
  ssbo.inner.m[dest[0]].el = value.el;
}

void assign_and_preserve_padding_ssbo_inner_m(strided_arr value[2]) {
  {
    for(uint i = 0u; (i < 2u); i = (i + 1u)) {
      uint tint_symbol_5[1] = uint[1](i);
      assign_and_preserve_padding_1_ssbo_inner_m_X(tint_symbol_5, value[i]);
    }
  }
}

void f_1() {
  mat2 tint_symbol = arr_to_mat2x2_stride_16(ssbo.inner.m);
  strided_arr tint_symbol_1[2] = mat2x2_stride_16_to_arr(tint_symbol);
  assign_and_preserve_padding_ssbo_inner_m(tint_symbol_1);
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
