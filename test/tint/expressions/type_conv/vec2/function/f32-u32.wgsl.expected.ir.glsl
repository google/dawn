#version 310 es

float t = 0.0f;
vec2 m() {
  t = 1.0f;
  return vec2(t);
}
uvec2 tint_v2f32_to_v2u32(vec2 value) {
  uvec2 v_1 = uvec2(value);
  uint v_2 = (((value >= vec2(0.0f)).x) ? (v_1.x) : (uvec2(0u).x));
  uvec2 v_3 = uvec2(v_2, (((value >= vec2(0.0f)).y) ? (v_1.y) : (uvec2(0u).y)));
  uint v_4 = (((value <= vec2(4294967040.0f)).x) ? (v_3.x) : (uvec2(4294967295u).x));
  return uvec2(v_4, (((value <= vec2(4294967040.0f)).y) ? (v_3.y) : (uvec2(4294967295u).y)));
}
void f() {
  uvec2 v = tint_v2f32_to_v2u32(m());
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
