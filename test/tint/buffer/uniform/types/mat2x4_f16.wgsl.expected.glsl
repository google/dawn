#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

layout(binding = 0, std140) uniform u_block_std140_ubo {
  f16vec4 inner_0;
  f16vec4 inner_1;
} u;

layout(binding = 1, std430) buffer u_block_ssbo {
  f16mat2x4 inner;
} s;

f16mat2x4 load_u_inner() {
  return f16mat2x4(u.inner_0, u.inner_1);
}

void tint_symbol() {
  f16mat2x4 x = load_u_inner();
  s.inner = x;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
