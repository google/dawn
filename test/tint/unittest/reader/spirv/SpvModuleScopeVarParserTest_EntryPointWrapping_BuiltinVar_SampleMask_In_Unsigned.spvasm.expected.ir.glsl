SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


uint x_1[1] = uint[1](0u);
void main_1() {
}
void main(uint x_1_param) {
  x_1[0] = x_1_param;
  main_1();
}
error: Error parsing GLSL shader:
ERROR: 0:9: 'main' : function cannot take any parameter(s) 
ERROR: 0:9: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
