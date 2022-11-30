#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

layout(binding = 0, std140) uniform u_block_std140_ubo {
  f16vec3 inner_0;
  f16vec3 inner_1;
  f16vec3 inner_2;
} u;

void a(f16mat3 m) {
}

void b(f16vec3 v) {
}

void c(float16_t f_1) {
}

f16mat3 load_u_inner() {
  return f16mat3(u.inner_0, u.inner_1, u.inner_2);
}

void f() {
  a(load_u_inner());
  b(u.inner_1);
  b(u.inner_1.zxy);
  c(u.inner_1[0u]);
  c(u.inner_1.zxy[0u]);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
