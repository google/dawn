#version 310 es

vec4 u = vec4(1.0f);
uvec4 tint_v4f32_to_v4u32(vec4 value) {
  uvec4 v_1 = uvec4(value);
  uint v_2 = (((value >= vec4(0.0f)).x) ? (v_1.x) : (uvec4(0u).x));
  uint v_3 = (((value >= vec4(0.0f)).y) ? (v_1.y) : (uvec4(0u).y));
  uint v_4 = (((value >= vec4(0.0f)).z) ? (v_1.z) : (uvec4(0u).z));
  uvec4 v_5 = uvec4(v_2, v_3, v_4, (((value >= vec4(0.0f)).w) ? (v_1.w) : (uvec4(0u).w)));
  uint v_6 = (((value <= vec4(4294967040.0f)).x) ? (v_5.x) : (uvec4(4294967295u).x));
  uint v_7 = (((value <= vec4(4294967040.0f)).y) ? (v_5.y) : (uvec4(4294967295u).y));
  uint v_8 = (((value <= vec4(4294967040.0f)).z) ? (v_5.z) : (uvec4(4294967295u).z));
  return uvec4(v_6, v_7, v_8, (((value <= vec4(4294967040.0f)).w) ? (v_5.w) : (uvec4(4294967295u).w)));
}
void f() {
  uvec4 v = tint_v4f32_to_v4u32(u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
