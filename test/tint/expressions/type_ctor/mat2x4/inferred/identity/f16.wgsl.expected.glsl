#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

f16mat2x4 m = f16mat2x4(f16vec4(0.0hf, 1.0hf, 2.0hf, 3.0hf), f16vec4(4.0hf, 5.0hf, 6.0hf, 7.0hf));
layout(binding = 0, std430) buffer tint_symbol_block_ssbo {
  f16mat2x4 inner;
} tint_symbol;

void f() {
  tint_symbol.inner = f16mat2x4(m);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
