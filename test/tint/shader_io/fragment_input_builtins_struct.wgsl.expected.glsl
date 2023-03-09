#version 310 es
#extension GL_OES_sample_variables : require
precision highp float;

struct FragmentInputs {
  vec4 position;
  bool front_facing;
  uint sample_index;
  uint sample_mask;
};

void tint_symbol(FragmentInputs inputs) {
  if (inputs.front_facing) {
    vec4 foo = inputs.position;
    uint bar = (inputs.sample_index + inputs.sample_mask);
  }
}

void main() {
  FragmentInputs tint_symbol_1 = FragmentInputs(gl_FragCoord, gl_FrontFacing, uint(gl_SampleID), uint(gl_SampleMaskIn[0]));
  tint_symbol(tint_symbol_1);
  return;
}
