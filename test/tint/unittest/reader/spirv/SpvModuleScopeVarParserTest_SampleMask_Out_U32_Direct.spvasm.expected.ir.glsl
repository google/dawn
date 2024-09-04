SKIP: FAILED

#version 310 es

struct main_out {
  uint x_1_1;
};
precision highp float;
precision highp int;


uint x_1[1] = uint[1](0u);
void main_1() {
  x_1[0] = 0u;
}
main_out main() {
  main_1();
  return main_out(x_1[0]);
}
error: Error parsing GLSL shader:
ERROR: 0:14: 'structure' :  entry point cannot return a value
ERROR: 0:14: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
