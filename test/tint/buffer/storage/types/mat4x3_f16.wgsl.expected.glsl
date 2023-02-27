#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

layout(binding = 0, std430) buffer tint_symbol_block_ssbo {
  f16mat4x3 inner;
} tint_symbol;

layout(binding = 1, std430) buffer tint_symbol_block_ssbo_1 {
  f16mat4x3 inner;
} tint_symbol_1;

void assign_and_preserve_padding_tint_symbol_1(f16mat4x3 value) {
  tint_symbol_1.inner[0] = value[0u];
  tint_symbol_1.inner[1] = value[1u];
  tint_symbol_1.inner[2] = value[2u];
  tint_symbol_1.inner[3] = value[3u];
}

void tint_symbol_2() {
  assign_and_preserve_padding_tint_symbol_1(tint_symbol.inner);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol_2();
  return;
}
