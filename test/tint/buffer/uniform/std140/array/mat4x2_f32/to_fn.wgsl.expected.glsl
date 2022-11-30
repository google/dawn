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

void a(mat4x2 a_1[4]) {
}

void b(mat4x2 m) {
}

void c(vec2 v) {
}

void d(float f_1) {
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
  a(conv_arr4_mat4x2_f32(u.inner));
  b(conv_mat4x2_f32(u.inner[1u]));
  c(u.inner[1u].col0.yx);
  d(u.inner[1u].col0.yx[0u]);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
