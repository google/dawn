#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

f16vec3 u = f16vec3(1.0hf);
uvec3 tint_v3f16_to_v3u32(f16vec3 value) {
  uvec3 v_1 = uvec3(value);
  uint v_2 = (((value >= f16vec3(0.0hf)).x) ? (v_1.x) : (uvec3(0u).x));
  uint v_3 = (((value >= f16vec3(0.0hf)).y) ? (v_1.y) : (uvec3(0u).y));
  uvec3 v_4 = uvec3(v_2, v_3, (((value >= f16vec3(0.0hf)).z) ? (v_1.z) : (uvec3(0u).z)));
  uint v_5 = (((value <= f16vec3(65504.0hf)).x) ? (v_4.x) : (uvec3(4294967295u).x));
  uint v_6 = (((value <= f16vec3(65504.0hf)).y) ? (v_4.y) : (uvec3(4294967295u).y));
  return uvec3(v_5, v_6, (((value <= f16vec3(65504.0hf)).z) ? (v_4.z) : (uvec3(4294967295u).z)));
}
void f() {
  uvec3 v = tint_v3f16_to_v3u32(u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
