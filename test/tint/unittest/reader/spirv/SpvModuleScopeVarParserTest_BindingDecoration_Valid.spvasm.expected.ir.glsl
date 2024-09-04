SKIP: FAILED

#version 310 es

struct S {
  uint field0;
  float field1;
  uint field2[2];
};
precision highp float;
precision highp int;


S x_1;
void main_1() {
}
void main() {
  main_1();
}
error: Error parsing GLSL shader:
ERROR: 0:5: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:5: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
