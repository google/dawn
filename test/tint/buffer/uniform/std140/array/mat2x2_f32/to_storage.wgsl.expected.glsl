#version 310 es

struct mat2x2_f32 {
  vec2 col0;
  vec2 col1;
};

layout(binding = 0, std140) uniform u_block_std140_ubo {
  mat2x2_f32 inner[4];
} u;

layout(binding = 1, std430) buffer u_block_ssbo {
  mat2 inner[4];
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
  s.inner = conv_arr4_mat2x2_f32(u.inner);
  s.inner[1] = conv_mat2x2_f32(u.inner[2u]);
  s.inner[1][0] = u.inner[0u].col1.yx;
  s.inner[1][0].x = u.inner[0u].col1[0u];
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
