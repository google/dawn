#version 310 es

float t = 0.0f;
vec3 m() {
  t = 1.0f;
  return vec3(t);
}
uvec3 tint_v3f32_to_v3u32(vec3 value) {
  uvec3 v_1 = uvec3(value);
  uint v_2 = (((value >= vec3(0.0f)).x) ? (v_1.x) : (uvec3(0u).x));
  uint v_3 = (((value >= vec3(0.0f)).y) ? (v_1.y) : (uvec3(0u).y));
  uvec3 v_4 = uvec3(v_2, v_3, (((value >= vec3(0.0f)).z) ? (v_1.z) : (uvec3(0u).z)));
  uint v_5 = (((value <= vec3(4294967040.0f)).x) ? (v_4.x) : (uvec3(4294967295u).x));
  uint v_6 = (((value <= vec3(4294967040.0f)).y) ? (v_4.y) : (uvec3(4294967295u).y));
  return uvec3(v_5, v_6, (((value <= vec3(4294967040.0f)).z) ? (v_4.z) : (uvec3(4294967295u).z)));
}
void f() {
  uvec3 v = tint_v3f32_to_v3u32(m());
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
