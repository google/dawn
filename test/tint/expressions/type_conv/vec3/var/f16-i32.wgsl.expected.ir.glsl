#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

f16vec3 u = f16vec3(1.0hf);
ivec3 tint_v3f16_to_v3i32(f16vec3 value) {
  ivec3 v_1 = ivec3(value);
  bvec3 v_2 = greaterThanEqual(value, f16vec3(-65504.0hf));
  int v_3 = ((v_2.x) ? (v_1.x) : (ivec3((-2147483647 - 1)).x));
  int v_4 = ((v_2.y) ? (v_1.y) : (ivec3((-2147483647 - 1)).y));
  ivec3 v_5 = ivec3(v_3, v_4, ((v_2.z) ? (v_1.z) : (ivec3((-2147483647 - 1)).z)));
  bvec3 v_6 = lessThanEqual(value, f16vec3(65504.0hf));
  int v_7 = ((v_6.x) ? (v_5.x) : (ivec3(2147483647).x));
  int v_8 = ((v_6.y) ? (v_5.y) : (ivec3(2147483647).y));
  return ivec3(v_7, v_8, ((v_6.z) ? (v_5.z) : (ivec3(2147483647).z)));
}
void f() {
  ivec3 v = tint_v3f16_to_v3i32(u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
