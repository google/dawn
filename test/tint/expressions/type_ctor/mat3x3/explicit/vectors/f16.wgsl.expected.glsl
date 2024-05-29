#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

f16mat3 m = f16mat3(f16vec3(0.0hf, 1.0hf, 2.0hf), f16vec3(3.0hf, 4.0hf, 5.0hf), f16vec3(6.0hf, 7.0hf, 8.0hf));
layout(binding = 0, std430) buffer tint_symbol_block_ssbo {
  f16mat3 inner;
} tint_symbol;

void assign_and_preserve_padding_tint_symbol_inner(f16mat3 value) {
  tint_symbol.inner[0] = value[0u];
  tint_symbol.inner[1] = value[1u];
  tint_symbol.inner[2] = value[2u];
}

void f() {
  assign_and_preserve_padding_tint_symbol_inner(m);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
