#version 310 es

vec2 u = vec2(1.0f);
uvec2 tint_v2f32_to_v2u32(vec2 value) {
  uvec2 v_1 = uvec2(value);
  bvec2 v_2 = greaterThanEqual(value, vec2(0.0f));
  uint v_3 = ((v_2.x) ? (v_1.x) : (uvec2(0u).x));
  uvec2 v_4 = uvec2(v_3, ((v_2.y) ? (v_1.y) : (uvec2(0u).y)));
  bvec2 v_5 = lessThanEqual(value, vec2(4294967040.0f));
  uint v_6 = ((v_5.x) ? (v_4.x) : (uvec2(4294967295u).x));
  return uvec2(v_6, ((v_5.y) ? (v_4.y) : (uvec2(4294967295u).y)));
}
void f() {
  uvec2 v = tint_v2f32_to_v2u32(u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
