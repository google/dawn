#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

layout(binding = 0, std430) buffer tint_symbol_block_ssbo {
  f16mat2x3 inner;
} tint_symbol;

void assign_and_preserve_padding_tint_symbol_inner(f16mat2x3 value) {
  tint_symbol.inner[0] = value[0u];
  tint_symbol.inner[1] = value[1u];
}

void f() {
  f16mat2x3 m = f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf));
  assign_and_preserve_padding_tint_symbol_inner(f16mat2x3(m));
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
