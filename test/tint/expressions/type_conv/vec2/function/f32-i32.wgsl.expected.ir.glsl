#version 310 es

float t = 0.0f;
vec2 m() {
  t = 1.0f;
  return vec2(t);
}
ivec2 tint_v2f32_to_v2i32(vec2 value) {
  ivec2 v_1 = ivec2(value);
  ivec2 v_2 = mix(ivec2((-2147483647 - 1)), v_1, greaterThanEqual(value, vec2(-2147483648.0f)));
  return mix(ivec2(2147483647), v_2, lessThanEqual(value, vec2(2147483520.0f)));
}
void f() {
  ivec2 v = tint_v2f32_to_v2i32(m());
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
