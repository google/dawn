#version 310 es

float t = 0.0f;
vec2 m() {
  t = 1.0f;
  return vec2(t);
}
uvec2 tint_v2f32_to_v2u32(vec2 value) {
  uvec2 v_1 = uvec2(value);
  uvec2 v_2 = mix(uvec2(0u), v_1, greaterThanEqual(value, vec2(0.0f)));
  return mix(uvec2(4294967295u), v_2, lessThanEqual(value, vec2(4294967040.0f)));
}
void f() {
  uvec2 v = tint_v2f32_to_v2u32(m());
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
