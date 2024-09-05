#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_2_1_ssbo {
  vec3 tint_symbol_1;
} v_1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  vec3 v = vec3(0.0f);
  float scalar = v.y;
  vec2 swizzle2 = v.xz;
  vec3 swizzle3 = v.xzy;
  vec3 v_2 = vec3(scalar);
  v_1.tint_symbol_1 = ((v_2 + vec3(swizzle2, 1.0f)) + swizzle3);
}
