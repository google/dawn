SKIP: FAILED

#version 310 es

vec4 main(int loc0, uint loc1, float loc2, vec4 loc3) {
  int i = loc0;
  uint u = loc1;
  float f = loc2;
  vec4 v = loc3;
  return vec4(0.0f);
}
error: Error parsing GLSL shader:
ERROR: 0:3: 'main' : function cannot take any parameter(s) 
ERROR: 0:3: 'float' :  entry point cannot return a value
ERROR: 0:3: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
