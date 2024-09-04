SKIP: FAILED

#version 310 es

struct S {
  uint field0;
  uint field1[];
};
precision highp float;
precision highp int;


S myvar;
void main_1() {
}
void main() {
  main_1();
}
error: Error parsing GLSL shader:
ERROR: 0:5: '' : array size required 
ERROR: 0:6: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
