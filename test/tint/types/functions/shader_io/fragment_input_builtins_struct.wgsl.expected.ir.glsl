SKIP: FAILED

#version 310 es
#extension GL_OES_sample_variables: require
precision highp float;
precision highp int;


struct FragmentInputs {
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
void main() {
  tint_symbol_inner(FragmentInputs(gl_FragCoord, gl_FrontFacing, gl_SampleID, gl_SampleMaskIn));
}
error: Error parsing GLSL shader:
ERROR: 0:21: 'constructor' : array argument must be sized 
ERROR: 0:21: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
