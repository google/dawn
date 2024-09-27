#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

float16_t t = 0.0hf;
f16vec3 m() {
  t = 1.0hf;
  return f16vec3(t);
}
ivec3 tint_v3f16_to_v3i32(f16vec3 value) {
  ivec3 v_1 = ivec3(value);
  ivec3 v_2 = mix(ivec3((-2147483647 - 1)), v_1, greaterThanEqual(value, f16vec3(-65504.0hf)));
  return mix(ivec3(2147483647), v_2, lessThanEqual(value, f16vec3(65504.0hf)));
}
void f() {
  ivec3 v = tint_v3f16_to_v3i32(m());
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
