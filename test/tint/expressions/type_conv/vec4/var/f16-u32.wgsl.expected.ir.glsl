#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

f16vec4 u = f16vec4(1.0hf);
uvec4 tint_v4f16_to_v4u32(f16vec4 value) {
  uvec4 v_1 = uvec4(value);
  uint v_2 = (((value >= f16vec4(0.0hf)).x) ? (v_1.x) : (uvec4(0u).x));
  uint v_3 = (((value >= f16vec4(0.0hf)).y) ? (v_1.y) : (uvec4(0u).y));
  uint v_4 = (((value >= f16vec4(0.0hf)).z) ? (v_1.z) : (uvec4(0u).z));
  uvec4 v_5 = uvec4(v_2, v_3, v_4, (((value >= f16vec4(0.0hf)).w) ? (v_1.w) : (uvec4(0u).w)));
  uint v_6 = (((value <= f16vec4(65504.0hf)).x) ? (v_5.x) : (uvec4(4294967295u).x));
  uint v_7 = (((value <= f16vec4(65504.0hf)).y) ? (v_5.y) : (uvec4(4294967295u).y));
  uint v_8 = (((value <= f16vec4(65504.0hf)).z) ? (v_5.z) : (uvec4(4294967295u).z));
  return uvec4(v_6, v_7, v_8, (((value <= f16vec4(65504.0hf)).w) ? (v_5.w) : (uvec4(4294967295u).w)));
}
void f() {
  uvec4 v = tint_v4f16_to_v4u32(u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
