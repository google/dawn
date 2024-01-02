#version 310 es

struct mat2x2_f32 {
  vec2 col0;
  vec2 col1;
};

layout(binding = 0, std140) uniform a_block_std140_ubo {
  mat2x2_f32 inner[4];
} a;

layout(binding = 1, std430) buffer s_block_ssbo {
  float inner;
} s;

mat2 conv_mat2x2_f32(mat2x2_f32 val) {
  return mat2(val.col0, val.col1);
}

mat2[4] conv_arr4_mat2x2_f32(mat2x2_f32 val[4]) {
  mat2 arr[4] = mat2[4](mat2(0.0f, 0.0f, 0.0f, 0.0f), mat2(0.0f, 0.0f, 0.0f, 0.0f), mat2(0.0f, 0.0f, 0.0f, 0.0f), mat2(0.0f, 0.0f, 0.0f, 0.0f));
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      arr[i] = conv_mat2x2_f32(val[i]);
    }
  }
  return arr;
}

void f() {
  mat2 p_a[4] = conv_arr4_mat2x2_f32(a.inner);
  mat2 p_a_2 = conv_mat2x2_f32(a.inner[2u]);
  vec2 p_a_2_1 = a.inner[2u].col1;
  mat2 l_a[4] = conv_arr4_mat2x2_f32(a.inner);
  mat2 l_a_i = conv_mat2x2_f32(a.inner[2u]);
  vec2 l_a_i_i = a.inner[2u].col1;
  s.inner = (((a.inner[2u].col1[0u] + l_a[0][0].x) + l_a_i[0].x) + l_a_i_i.x);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
