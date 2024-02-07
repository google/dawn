#version 310 es
#extension GL_OES_sample_variables : require
precision highp float;
precision highp int;

layout(location = 0) flat in int loc0_1;
layout(location = 1) flat in uint loc1_1;
layout(location = 3) in vec4 loc3_1;
layout(location = 2) in float loc2_1;
struct FragmentInputs0 {
  vec4 position;
  int loc0;
};

struct FragmentInputs1 {
  vec4 loc3;
  uint sample_mask;
};

void tint_symbol(FragmentInputs0 inputs0, bool front_facing, uint loc1, uint sample_index, FragmentInputs1 inputs1, float loc2) {
  if (front_facing) {
    vec4 foo = inputs0.position;
    uint bar = (sample_index + inputs1.sample_mask);
    int i = inputs0.loc0;
    uint u = loc1;
    float f = loc2;
    vec4 v = inputs1.loc3;
  }
}

void main() {
  FragmentInputs0 tint_symbol_1 = FragmentInputs0(gl_FragCoord, loc0_1);
  FragmentInputs1 tint_symbol_2 = FragmentInputs1(loc3_1, uint(gl_SampleMaskIn[0]));
  tint_symbol(tint_symbol_1, gl_FrontFacing, loc1_1, uint(gl_SampleID), tint_symbol_2, loc2_1);
  return;
}
