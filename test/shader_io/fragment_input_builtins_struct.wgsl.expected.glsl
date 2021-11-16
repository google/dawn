SKIP: FAILED

#version 310 es
precision mediump float;

struct FragmentInputs {
  vec4 position;
  bool front_facing;
  uint sample_index;
  uint sample_mask;
};
struct tint_symbol_2 {
  vec4 position;
  bool front_facing;
  uint sample_index;
  uint sample_mask;
};

void tint_symbol_inner(FragmentInputs inputs) {
  if (inputs.front_facing) {
    vec4 foo = inputs.position;
    uint bar = (inputs.sample_index + inputs.sample_mask);
  }
}

void tint_symbol(tint_symbol_2 tint_symbol_1) {
  FragmentInputs tint_symbol_3 = FragmentInputs(tint_symbol_1.position, tint_symbol_1.front_facing, tint_symbol_1.sample_index, tint_symbol_1.sample_mask);
  tint_symbol_inner(tint_symbol_3);
  return;
}
void main() {
  tint_symbol_2 inputs;
  inputs.position = gl_FragCoord;
  inputs.front_facing = gl_FrontFacing;
  inputs.sample_index = uint(gl_SampleID);
  inputs.sample_mask = uint(gl_SampleMask);
  tint_symbol(inputs);
}


Error parsing GLSL shader:
ERROR: 0:33: 'gl_SampleID' : required extension not requested: GL_OES_sample_variables
ERROR: 0:33: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



