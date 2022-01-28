SKIP: FAILED

#version 310 es
precision mediump float;

void tint_symbol(vec4 position, bool front_facing, uint sample_index, uint sample_mask) {
  if (front_facing) {
    vec4 foo = position;
    uint bar = (sample_index + sample_mask);
  }
}

void main() {
  tint_symbol(gl_FragCoord, gl_FrontFacing, uint(gl_SampleID), uint(gl_SampleMask[0]));
  return;
}
Error parsing GLSL shader:
ERROR: 0:12: 'gl_SampleID' : required extension not requested: GL_OES_sample_variables
ERROR: 0:12: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



