SKIP: FAILED

#version 310 es
#extension GL_OES_sample_variables: require
precision highp float;
precision highp int;

uint x_1 = 0u;
void main_1() {
  uint x_2 = x_1;
}
void tint_symbol_inner(uint x_1_param) {
  x_1 = x_1_param;
  main_1();
}
void main() {
  tint_symbol_inner(gl_SampleID);
}
error: Error parsing GLSL shader:
ERROR: 0:15: 'tint_symbol_inner' : no matching overloaded function found 
ERROR: 0:15: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
