SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


float x_1 = 0.0f;
float x_2 = 0.0f;
float x_3 = 0.0f;
float x_4 = 0.0f;
float x_5 = 0.0f;
float x_6 = 0.0f;
void main_1() {
}
void main(float x_1_param, float x_2_param, float x_3_param, float x_4_param, float x_5_param, float x_6_param) {
  x_1 = x_1_param;
  x_2 = x_2_param;
  x_3 = x_3_param;
  x_4 = x_4_param;
  x_5 = x_5_param;
  x_6 = x_6_param;
  main_1();
}
error: Error parsing GLSL shader:
ERROR: 0:14: 'main' : function cannot take any parameter(s) 
ERROR: 0:14: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
