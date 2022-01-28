SKIP: FAILED

#version 310 es
precision mediump float;

float main1() {
  return 1.0f;
}

void main() {
  float inner_result = main1();
  gl_FragDepth = inner_result;
  return;
}
#version 310 es
precision mediump float;

uint main2() {
  return 1u;
}

void main() {
  uint inner_result = main2();
  gl_SampleMask[0] = inner_result;
  return;
}
Error parsing GLSL shader:
ERROR: 0:10: 'gl_SampleMask' : required extension not requested: GL_OES_sample_variables
ERROR: 0:10: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



