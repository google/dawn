#version 310 es

layout(binding = 0, std140)
uniform tint_symbol_2_1_ubo {
  uint tint_symbol_1;
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  vec3 v1 = vec3(0.0f);
  v1[v.tint_symbol_1] = 1.0f;
}
