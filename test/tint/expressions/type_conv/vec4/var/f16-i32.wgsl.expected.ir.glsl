#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

f16vec4 u = f16vec4(1.0hf);
ivec4 tint_v4f16_to_v4i32(f16vec4 value) {
  ivec4 v_1 = ivec4(value);
  bvec4 v_2 = greaterThanEqual(value, f16vec4(-65504.0hf));
  int v_3 = ((v_2.x) ? (v_1.x) : (ivec4((-2147483647 - 1)).x));
  int v_4 = ((v_2.y) ? (v_1.y) : (ivec4((-2147483647 - 1)).y));
  int v_5 = ((v_2.z) ? (v_1.z) : (ivec4((-2147483647 - 1)).z));
  ivec4 v_6 = ivec4(v_3, v_4, v_5, ((v_2.w) ? (v_1.w) : (ivec4((-2147483647 - 1)).w)));
  bvec4 v_7 = lessThanEqual(value, f16vec4(65504.0hf));
  int v_8 = ((v_7.x) ? (v_6.x) : (ivec4(2147483647).x));
  int v_9 = ((v_7.y) ? (v_6.y) : (ivec4(2147483647).y));
  int v_10 = ((v_7.z) ? (v_6.z) : (ivec4(2147483647).z));
  return ivec4(v_8, v_9, v_10, ((v_7.w) ? (v_6.w) : (ivec4(2147483647).w)));
}
void f() {
  ivec4 v = tint_v4f16_to_v4i32(u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
