#version 310 es

struct mat4x2_f32 {
  vec2 col0;
  vec2 col1;
  vec2 col2;
  vec2 col3;
};

layout(binding = 0, std140) uniform u_block_std140_ubo {
  mat4x2_f32 inner[4];
} u;

layout(binding = 1, std430) buffer s_block_ssbo {
  float inner;
} s;

float a(mat4x2 a_1[4]) {
  return a_1[0][0].x;
}

float b(mat4x2 m) {
  return m[0].x;
}

float c(vec2 v) {
  return v.x;
}

float d(float f_1) {
  return f_1;
}

mat4x2 conv_mat4x2_f32(mat4x2_f32 val) {
  return mat4x2(val.col0, val.col1, val.col2, val.col3);
}

mat4x2[4] conv_arr4_mat4x2_f32(mat4x2_f32 val[4]) {
  mat4x2 arr[4] = mat4x2[4](mat4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f), mat4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f), mat4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f), mat4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f));
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      arr[i] = conv_mat4x2_f32(val[i]);
    }
  }
  return arr;
}

void f() {
  float tint_symbol = a(conv_arr4_mat4x2_f32(u.inner));
  float tint_symbol_1 = b(conv_mat4x2_f32(u.inner[1u]));
  float tint_symbol_2 = c(u.inner[1u].col0.yx);
  float tint_symbol_3 = d(u.inner[1u].col0.yx[0u]);
  s.inner = (((tint_symbol + tint_symbol_1) + tint_symbol_2) + tint_symbol_3);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
