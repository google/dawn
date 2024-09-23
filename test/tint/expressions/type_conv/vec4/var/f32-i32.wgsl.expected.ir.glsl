#version 310 es

vec4 u = vec4(1.0f);
ivec4 tint_v4f32_to_v4i32(vec4 value) {
  ivec4 v_1 = ivec4(value);
  bvec4 v_2 = greaterThanEqual(value, vec4(-2147483648.0f));
  int v_3 = ((v_2.x) ? (v_1.x) : (ivec4((-2147483647 - 1)).x));
  int v_4 = ((v_2.y) ? (v_1.y) : (ivec4((-2147483647 - 1)).y));
  int v_5 = ((v_2.z) ? (v_1.z) : (ivec4((-2147483647 - 1)).z));
  ivec4 v_6 = ivec4(v_3, v_4, v_5, ((v_2.w) ? (v_1.w) : (ivec4((-2147483647 - 1)).w)));
  bvec4 v_7 = lessThanEqual(value, vec4(2147483520.0f));
  int v_8 = ((v_7.x) ? (v_6.x) : (ivec4(2147483647).x));
  int v_9 = ((v_7.y) ? (v_6.y) : (ivec4(2147483647).y));
  int v_10 = ((v_7.z) ? (v_6.z) : (ivec4(2147483647).z));
  return ivec4(v_8, v_9, v_10, ((v_7.w) ? (v_6.w) : (ivec4(2147483647).w)));
}
void f() {
  ivec4 v = tint_v4f32_to_v4i32(u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
