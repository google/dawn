SKIP: FAILED

#version 310 es

struct main_out {
  uint x_2_1;
  uint x_4_1;
};
precision highp float;
precision highp int;


uint x_1 = 0u;
uint x_2 = 0u;
uint x_3 = 0u;
uint x_4 = 0u;
void main_1() {
}
main_out main(uint x_1_param, uint x_3_param) {
  x_1 = x_1_param;
  x_3 = x_3_param;
  main_1();
  return main_out(x_2, x_4);
}
error: Error parsing GLSL shader:
ERROR: 0:17: 'main' : function cannot take any parameter(s) 
ERROR: 0:17: 'structure' :  entry point cannot return a value
ERROR: 0:17: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
