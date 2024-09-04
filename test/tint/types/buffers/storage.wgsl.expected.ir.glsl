SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


float weights[];
void main() {
  float a = weights[0];
}
error: Error parsing GLSL shader:
ERROR: 0:6: '' : array size required 
ERROR: 1 compilation errors.  No code generated.




tint executable returned error: exit status 1
