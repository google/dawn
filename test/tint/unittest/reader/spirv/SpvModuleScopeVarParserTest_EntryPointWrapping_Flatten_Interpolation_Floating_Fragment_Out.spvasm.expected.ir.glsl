SKIP: FAILED

#version 310 es

struct S {
  float field0;
  float field1;
  float field2;
  float field3;
  float field4;
  float field5;
};

struct main_out {
  float x_1_1;
  float x_1_2;
  float x_1_3;
  float x_1_4;
  float x_1_5;
  float x_1_6;
};
precision highp float;
precision highp int;


S x_1 = S(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
void main_1() {
}
main_out main() {
  main_1();
  return main_out(x_1.field0, x_1.field1, x_1.field2, x_1.field3, x_1.field4, x_1.field5);
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
