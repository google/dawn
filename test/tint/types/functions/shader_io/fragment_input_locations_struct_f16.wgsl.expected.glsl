#version 310 es
#extension GL_AMD_gpu_shader_half_float : require
precision highp float;

layout(location = 0) flat in int loc0_1;
layout(location = 1) flat in uint loc1_1;
layout(location = 2) in float loc2_1;
layout(location = 3) in vec4 loc3_1;
layout(location = 4) in float16_t loc4_1;
layout(location = 5) in f16vec3 loc5_1;
struct FragmentInputs {
  int loc0;
  uint loc1;
  float loc2;
  vec4 loc3;
  float16_t loc4;
  f16vec3 loc5;
};

void tint_symbol(FragmentInputs inputs) {
  int i = inputs.loc0;
  uint u = inputs.loc1;
  float f = inputs.loc2;
  vec4 v = inputs.loc3;
  float16_t x = inputs.loc4;
  f16vec3 y = inputs.loc5;
}

void main() {
  FragmentInputs tint_symbol_1 = FragmentInputs(loc0_1, loc1_1, loc2_1, loc3_1, loc4_1, loc5_1);
  tint_symbol(tint_symbol_1);
  return;
}
