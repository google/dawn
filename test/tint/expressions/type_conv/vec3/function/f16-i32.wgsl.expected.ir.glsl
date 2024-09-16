#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

float16_t t = 0.0hf;
f16vec3 m() {
  t = 1.0hf;
  return f16vec3(t);
}
ivec3 tint_v3f16_to_v3i32(f16vec3 value) {
  ivec3 v_1 = ivec3(value);
  int v_2 = (((value >= f16vec3(-65504.0hf)).x) ? (v_1.x) : (ivec3((-2147483647 - 1)).x));
  int v_3 = (((value >= f16vec3(-65504.0hf)).y) ? (v_1.y) : (ivec3((-2147483647 - 1)).y));
  ivec3 v_4 = ivec3(v_2, v_3, (((value >= f16vec3(-65504.0hf)).z) ? (v_1.z) : (ivec3((-2147483647 - 1)).z)));
  int v_5 = (((value <= f16vec3(65504.0hf)).x) ? (v_4.x) : (ivec3(2147483647).x));
  int v_6 = (((value <= f16vec3(65504.0hf)).y) ? (v_4.y) : (ivec3(2147483647).y));
  return ivec3(v_5, v_6, (((value <= f16vec3(65504.0hf)).z) ? (v_4.z) : (ivec3(2147483647).z)));
}
void f() {
  ivec3 v = tint_v3f16_to_v3i32(m());
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
