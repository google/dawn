SKIP: FAILED

#version 310 es

struct S {
  float field0;
  float field1;
};
precision highp float;
precision highp int;


float x_1[2] = float[2](0.0f, 0.0f);
S x_2 = S(0.0f, 0.0f);
void main_1() {
}
void main(float x_1_param, float x_1_param_1, float x_2_param, float x_2_param_1) {
  x_1[0] = x_1_param;
  x_1[1] = x_1_param_1;
  x_2.field0 = x_2_param;
  x_2.field1 = x_2_param_1;
  main_1();
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
