SKIP: FAILED

#version 310 es

struct S {
  bool field0;
  uint field1;
  int field2;
  float field3;
};
precision highp float;
precision highp int;


void main_1() {
  S x_11 = S(false, 0u, 0, 0.0f);
}
void main() {
  main_1();
}
error: Error parsing GLSL shader:
ERROR: 0:7: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:7: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
