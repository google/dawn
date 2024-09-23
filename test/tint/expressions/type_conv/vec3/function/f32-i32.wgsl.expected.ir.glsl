#version 310 es

float t = 0.0f;
vec3 m() {
  t = 1.0f;
  return vec3(t);
}
ivec3 tint_v3f32_to_v3i32(vec3 value) {
  ivec3 v_1 = ivec3(value);
  int v_2 = (((value >= vec3(-2147483648.0f)).x) ? (v_1.x) : (ivec3((-2147483647 - 1)).x));
  int v_3 = (((value >= vec3(-2147483648.0f)).y) ? (v_1.y) : (ivec3((-2147483647 - 1)).y));
  ivec3 v_4 = ivec3(v_2, v_3, (((value >= vec3(-2147483648.0f)).z) ? (v_1.z) : (ivec3((-2147483647 - 1)).z)));
  int v_5 = (((value <= vec3(2147483520.0f)).x) ? (v_4.x) : (ivec3(2147483647).x));
  int v_6 = (((value <= vec3(2147483520.0f)).y) ? (v_4.y) : (ivec3(2147483647).y));
  return ivec3(v_5, v_6, (((value <= vec3(2147483520.0f)).z) ? (v_4.z) : (ivec3(2147483647).z)));
}
void f() {
  ivec3 v = tint_v3f32_to_v3i32(m());
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
