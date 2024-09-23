SKIP: FAILED

#version 310 es
#extension GL_OES_sample_variables: require
precision highp float;
precision highp int;

void tint_symbol_inner(vec4 position, bool front_facing, uint sample_index, uint sample_mask) {
  if (front_facing) {
    vec4 foo = position;
    uint bar = (sample_index + sample_mask);
  }
}
void main() {
  tint_symbol_inner(gl_FragCoord, gl_FrontFacing, gl_SampleID, gl_SampleMaskIn);
}
error: Error parsing GLSL shader:
ERROR: 0:13: 'tint_symbol_inner' : no matching overloaded function found 
ERROR: 0:13: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
