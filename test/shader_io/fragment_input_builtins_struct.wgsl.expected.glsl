SKIP: FAILED

#version 310 es
precision mediump float;

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
  FragmentInputs tint_symbol_1 = FragmentInputs(gl_FragCoord, gl_FrontFacing, uint(gl_SampleID), uint(gl_SampleMask[0]));
  tint_symbol(tint_symbol_1);
  return;
}
Error parsing GLSL shader:
ERROR: 0:19: 'gl_SampleID' : required extension not requested: GL_OES_sample_variables
ERROR: 0:19: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



