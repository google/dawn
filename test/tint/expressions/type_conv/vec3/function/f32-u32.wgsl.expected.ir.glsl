#version 310 es

float t = 0.0f;
vec3 m() {
  t = 1.0f;
  return vec3(t);
}
uvec3 tint_v3f32_to_v3u32(vec3 value) {
  uvec3 v_1 = uvec3(value);
  bvec3 v_2 = greaterThanEqual(value, vec3(0.0f));
  uint v_3 = ((v_2.x) ? (v_1.x) : (uvec3(0u).x));
  uint v_4 = ((v_2.y) ? (v_1.y) : (uvec3(0u).y));
  uvec3 v_5 = uvec3(v_3, v_4, ((v_2.z) ? (v_1.z) : (uvec3(0u).z)));
  bvec3 v_6 = lessThanEqual(value, vec3(4294967040.0f));
  uint v_7 = ((v_6.x) ? (v_5.x) : (uvec3(4294967295u).x));
  uint v_8 = ((v_6.y) ? (v_5.y) : (uvec3(4294967295u).y));
  return uvec3(v_7, v_8, ((v_6.z) ? (v_5.z) : (uvec3(4294967295u).z)));
}
void f() {
  uvec3 v = tint_v3f32_to_v3u32(m());
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
