#version 310 es
#extension GL_OES_sample_variables: require
precision highp float;
precision highp int;


struct FragmentInputs0 {
  vec4 position;
  int loc0;
};

struct FragmentInputs1 {
  vec4 loc3;
  uint sample_mask;
};

layout(location = 0) flat in int tint_symbol_loc0_Input;
layout(location = 1) flat in uint tint_symbol_loc1_Input;
layout(location = 3) in vec4 tint_symbol_loc3_Input;
layout(location = 2) in float tint_symbol_loc2_Input;
void tint_symbol_inner(FragmentInputs0 inputs0, bool front_facing, uint loc1, uint sample_index, FragmentInputs1 inputs1, float loc2) {
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
  FragmentInputs0 v_1 = FragmentInputs0(gl_FragCoord, tint_symbol_loc0_Input);
  bool v_2 = gl_FrontFacing;
  uint v_3 = tint_symbol_loc1_Input;
  uint v_4 = uint(gl_SampleID);
  vec4 v_5 = tint_symbol_loc3_Input;
  FragmentInputs1 v_6 = FragmentInputs1(v_5, uint(gl_SampleMaskIn[0u]));
  tint_symbol_inner(v_1, v_2, v_3, v_4, v_6, tint_symbol_loc2_Input);
}
