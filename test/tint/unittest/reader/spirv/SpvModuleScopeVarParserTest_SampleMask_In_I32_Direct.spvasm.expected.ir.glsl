SKIP: FAILED

#version 310 es
#extension GL_OES_sample_variables: require
precision highp float;
precision highp int;

int x_1[1] = int[1](0);
void main_1() {
  int x_3 = x_1[0];
}
void tint_symbol_inner(uint x_1_param) {
  x_1[0] = int(x_1_param);
  main_1();
}
void main() {
  tint_symbol_inner(gl_SampleMaskIn);
}
error: Error parsing GLSL shader:
ERROR: 0:15: 'tint_symbol_inner' : no matching overloaded function found 
ERROR: 0:15: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
