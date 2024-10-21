#version 310 es
#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;


struct FragmentInputs {
  int loc0;
  uint loc1;
  float loc2;
  vec4 loc3;
  float16_t loc4;
  f16vec3 loc5;
};

layout(location = 0) flat in int tint_symbol_loc0_Input;
layout(location = 1) flat in uint tint_symbol_loc1_Input;
layout(location = 2) in float tint_symbol_loc2_Input;
layout(location = 3) in vec4 tint_symbol_loc3_Input;
layout(location = 4) in float16_t tint_symbol_loc4_Input;
layout(location = 5) in f16vec3 tint_symbol_loc5_Input;
void tint_symbol_inner(FragmentInputs inputs) {
  int i = inputs.loc0;
  uint u = inputs.loc1;
  float f = inputs.loc2;
  vec4 v = inputs.loc3;
  float16_t x = inputs.loc4;
  f16vec3 y = inputs.loc5;
}
void main() {
  tint_symbol_inner(FragmentInputs(tint_symbol_loc0_Input, tint_symbol_loc1_Input, tint_symbol_loc2_Input, tint_symbol_loc3_Input, tint_symbol_loc4_Input, tint_symbol_loc5_Input));
}
