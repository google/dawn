#version 310 es

float t = 0.0f;
vec2 m() {
  t = 1.0f;
  return vec2(t);
}
ivec2 tint_v2f32_to_v2i32(vec2 value) {
  ivec2 v_1 = ivec2(value);
  bvec2 v_2 = greaterThanEqual(value, vec2(-2147483648.0f));
  int v_3 = ((v_2.x) ? (v_1.x) : (ivec2((-2147483647 - 1)).x));
  ivec2 v_4 = ivec2(v_3, ((v_2.y) ? (v_1.y) : (ivec2((-2147483647 - 1)).y)));
  bvec2 v_5 = lessThanEqual(value, vec2(2147483520.0f));
  int v_6 = ((v_5.x) ? (v_4.x) : (ivec2(2147483647).x));
  return ivec2(v_6, ((v_5.y) ? (v_4.y) : (ivec2(2147483647).y)));
}
void f() {
  ivec2 v = tint_v2f32_to_v2i32(m());
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
