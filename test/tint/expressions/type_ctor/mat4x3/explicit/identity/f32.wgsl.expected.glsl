#version 310 es

mat4x3 m = mat4x3(vec3(0.0f, 1.0f, 2.0f), vec3(3.0f, 4.0f, 5.0f), vec3(6.0f, 7.0f, 8.0f), vec3(9.0f, 10.0f, 11.0f));
layout(binding = 0, std430) buffer tint_symbol_block_ssbo {
  mat4x3 inner;
} tint_symbol;

void assign_and_preserve_padding_tint_symbol(mat4x3 value) {
  tint_symbol.inner[0] = value[0u];
  tint_symbol.inner[1] = value[1u];
  tint_symbol.inner[2] = value[2u];
  tint_symbol.inner[3] = value[3u];
}

void f() {
  assign_and_preserve_padding_tint_symbol(mat4x3(m));
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
