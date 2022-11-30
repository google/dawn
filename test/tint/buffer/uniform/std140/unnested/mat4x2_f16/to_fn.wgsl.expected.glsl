#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

layout(binding = 0, std140) uniform u_block_std140_ubo {
  f16vec2 inner_0;
  f16vec2 inner_1;
  f16vec2 inner_2;
  f16vec2 inner_3;
} u;

void a(f16mat4x2 m) {
}

void b(f16vec2 v) {
}

void c(float16_t f_1) {
}

f16mat4x2 load_u_inner() {
  return f16mat4x2(u.inner_0, u.inner_1, u.inner_2, u.inner_3);
}

void f() {
  a(load_u_inner());
  b(u.inner_1);
  b(u.inner_1.yx);
  c(u.inner_1[0u]);
  c(u.inner_1.yx[0u]);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
