SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


void main(int loc0, uint loc1, float loc2, vec4 loc3) {
  int i = loc0;
  uint u = loc1;
  float f = loc2;
  vec4 v = loc3;
}
error: Error parsing GLSL shader:
ERROR: 0:6: 'main' : function cannot take any parameter(s) 
ERROR: 0:6: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
