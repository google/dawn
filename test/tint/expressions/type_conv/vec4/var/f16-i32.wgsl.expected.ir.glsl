#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

f16vec4 u = f16vec4(1.0hf);
ivec4 tint_v4f16_to_v4i32(f16vec4 value) {
  ivec4 v_1 = ivec4(value);
  int v_2 = (((value >= f16vec4(-65504.0hf)).x) ? (v_1.x) : (ivec4((-2147483647 - 1)).x));
  int v_3 = (((value >= f16vec4(-65504.0hf)).y) ? (v_1.y) : (ivec4((-2147483647 - 1)).y));
  int v_4 = (((value >= f16vec4(-65504.0hf)).z) ? (v_1.z) : (ivec4((-2147483647 - 1)).z));
  ivec4 v_5 = ivec4(v_2, v_3, v_4, (((value >= f16vec4(-65504.0hf)).w) ? (v_1.w) : (ivec4((-2147483647 - 1)).w)));
  int v_6 = (((value <= f16vec4(65504.0hf)).x) ? (v_5.x) : (ivec4(2147483647).x));
  int v_7 = (((value <= f16vec4(65504.0hf)).y) ? (v_5.y) : (ivec4(2147483647).y));
  int v_8 = (((value <= f16vec4(65504.0hf)).z) ? (v_5.z) : (ivec4(2147483647).z));
  return ivec4(v_6, v_7, v_8, (((value <= f16vec4(65504.0hf)).w) ? (v_5.w) : (ivec4(2147483647).w)));
}
void f() {
  ivec4 v = tint_v4f16_to_v4i32(u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
