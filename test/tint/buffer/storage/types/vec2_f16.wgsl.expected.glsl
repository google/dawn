#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

layout(binding = 0, std430) buffer tint_symbol_block_ssbo {
  f16vec2 inner;
} tint_symbol;

layout(binding = 1, std430) buffer tint_symbol_block_ssbo_1 {
  f16vec2 inner;
} tint_symbol_1;

void tint_symbol_2() {
  tint_symbol_1.inner = tint_symbol.inner;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol_2();
  return;
}
