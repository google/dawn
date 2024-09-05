#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_2_1_ssbo {
  float tint_symbol_1;
} v_1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat3 m = mat3(vec3(1.0f, 2.0f, 3.0f), vec3(4.0f, 5.0f, 6.0f), vec3(7.0f, 8.0f, 9.0f));
  vec3 v = m[1];
  float f = v[1];
  v_1.tint_symbol_1 = f;
}
