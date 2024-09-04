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
precision highp float;
precision highp int;


S x_1 = S(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
void main_1() {
}
void main(float x_1_param, float x_1_param_1, float x_1_param_2, float x_1_param_3, float x_1_param_4, float x_1_param_5) {
  x_1.field0 = x_1_param;
  x_1.field1 = x_1_param_1;
  x_1.field2 = x_1_param_2;
  x_1.field3 = x_1_param_3;
  x_1.field4 = x_1_param_4;
  x_1.field5 = x_1_param_5;
  main_1();
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
