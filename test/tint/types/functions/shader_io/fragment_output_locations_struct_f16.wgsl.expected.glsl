#version 310 es
#extension GL_AMD_gpu_shader_half_float : require
precision highp float;
precision highp int;

layout(location = 0) out int loc0_1;
layout(location = 1) out uint loc1_1;
layout(location = 2) out float loc2_1;
layout(location = 3) out vec4 loc3_1;
layout(location = 4) out float16_t loc4_1;
layout(location = 5) out f16vec3 loc5_1;
struct FragmentOutputs {
  int loc0;
  uint loc1;
  float loc2;
  vec4 loc3;
  float16_t loc4;
  f16vec3 loc5;
};

FragmentOutputs tint_symbol() {
  FragmentOutputs tint_symbol_1 = FragmentOutputs(1, 1u, 1.0f, vec4(1.0f, 2.0f, 3.0f, 4.0f), 2.25hf, f16vec3(3.0hf, 5.0hf, 8.0hf));
  return tint_symbol_1;
}

void main() {
  FragmentOutputs inner_result = tint_symbol();
  loc0_1 = inner_result.loc0;
  loc1_1 = inner_result.loc1;
  loc2_1 = inner_result.loc2;
  loc3_1 = inner_result.loc3;
  loc4_1 = inner_result.loc4;
  loc5_1 = inner_result.loc5;
  return;
}
