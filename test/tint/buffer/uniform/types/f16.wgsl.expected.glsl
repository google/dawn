#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

layout(binding = 0, std140) uniform u_block_ubo {
  float16_t inner;
} u;

void tint_symbol() {
  float16_t x = u.inner;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
