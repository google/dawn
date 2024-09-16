#version 310 es

float t = 0.0f;
vec4 m() {
  t = 1.0f;
  return vec4(t);
}
ivec4 tint_v4f32_to_v4i32(vec4 value) {
  ivec4 v_1 = ivec4(value);
  int v_2 = (((value >= vec4(-2147483648.0f)).x) ? (v_1.x) : (ivec4((-2147483647 - 1)).x));
  int v_3 = (((value >= vec4(-2147483648.0f)).y) ? (v_1.y) : (ivec4((-2147483647 - 1)).y));
  int v_4 = (((value >= vec4(-2147483648.0f)).z) ? (v_1.z) : (ivec4((-2147483647 - 1)).z));
  ivec4 v_5 = ivec4(v_2, v_3, v_4, (((value >= vec4(-2147483648.0f)).w) ? (v_1.w) : (ivec4((-2147483647 - 1)).w)));
  int v_6 = (((value <= vec4(2147483520.0f)).x) ? (v_5.x) : (ivec4(2147483647).x));
  int v_7 = (((value <= vec4(2147483520.0f)).y) ? (v_5.y) : (ivec4(2147483647).y));
  int v_8 = (((value <= vec4(2147483520.0f)).z) ? (v_5.z) : (ivec4(2147483647).z));
  return ivec4(v_6, v_7, v_8, (((value <= vec4(2147483520.0f)).w) ? (v_5.w) : (ivec4(2147483647).w)));
}
void f() {
  ivec4 v = tint_v4f32_to_v4i32(m());
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
