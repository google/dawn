#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

float16_t t = 0.0hf;
f16vec4 m() {
  t = 1.0hf;
  return f16vec4(t);
}
uvec4 tint_v4f16_to_v4u32(f16vec4 value) {
  uvec4 v_1 = uvec4(value);
  bvec4 v_2 = greaterThanEqual(value, f16vec4(0.0hf));
  uint v_3 = ((v_2.x) ? (v_1.x) : (uvec4(0u).x));
  uint v_4 = ((v_2.y) ? (v_1.y) : (uvec4(0u).y));
  uint v_5 = ((v_2.z) ? (v_1.z) : (uvec4(0u).z));
  uvec4 v_6 = uvec4(v_3, v_4, v_5, ((v_2.w) ? (v_1.w) : (uvec4(0u).w)));
  bvec4 v_7 = lessThanEqual(value, f16vec4(65504.0hf));
  uint v_8 = ((v_7.x) ? (v_6.x) : (uvec4(4294967295u).x));
  uint v_9 = ((v_7.y) ? (v_6.y) : (uvec4(4294967295u).y));
  uint v_10 = ((v_7.z) ? (v_6.z) : (uvec4(4294967295u).z));
  return uvec4(v_8, v_9, v_10, ((v_7.w) ? (v_6.w) : (uvec4(4294967295u).w)));
}
void f() {
  uvec4 v = tint_v4f16_to_v4u32(m());
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
