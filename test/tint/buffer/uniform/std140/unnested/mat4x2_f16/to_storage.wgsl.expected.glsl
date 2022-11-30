#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

layout(binding = 0, std140) uniform u_block_std140_ubo {
  f16vec2 inner_0;
  f16vec2 inner_1;
  f16vec2 inner_2;
  f16vec2 inner_3;
} u;

layout(binding = 1, std430) buffer u_block_ssbo {
  f16mat4x2 inner;
} s;

f16mat4x2 load_u_inner() {
  return f16mat4x2(u.inner_0, u.inner_1, u.inner_2, u.inner_3);
}

void f() {
  s.inner = load_u_inner();
  s.inner[1] = u.inner_0;
  s.inner[1] = u.inner_0.yx;
  s.inner[0][1] = u.inner_1[0u];
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
