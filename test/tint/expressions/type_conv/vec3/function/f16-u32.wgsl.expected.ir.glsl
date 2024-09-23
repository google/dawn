#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

float16_t t = 0.0hf;
f16vec3 m() {
  t = 1.0hf;
  return f16vec3(t);
}
uvec3 tint_v3f16_to_v3u32(f16vec3 value) {
  uvec3 v_1 = uvec3(value);
  bvec3 v_2 = greaterThanEqual(value, f16vec3(0.0hf));
  uint v_3 = ((v_2.x) ? (v_1.x) : (uvec3(0u).x));
  uint v_4 = ((v_2.y) ? (v_1.y) : (uvec3(0u).y));
  uvec3 v_5 = uvec3(v_3, v_4, ((v_2.z) ? (v_1.z) : (uvec3(0u).z)));
  bvec3 v_6 = lessThanEqual(value, f16vec3(65504.0hf));
  uint v_7 = ((v_6.x) ? (v_5.x) : (uvec3(4294967295u).x));
  uint v_8 = ((v_6.y) ? (v_5.y) : (uvec3(4294967295u).y));
  return uvec3(v_7, v_8, ((v_6.z) ? (v_5.z) : (uvec3(4294967295u).z)));
}
void f() {
  uvec3 v = tint_v3f16_to_v3u32(m());
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
