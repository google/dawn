SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;

uniform highp sampler1D x_20;
void main_1() {
  int v = int(0);
  vec4 x_125 = texelFetch(x_20, v, int(0));
}
void main() {
  main_1();
}
error: Error parsing GLSL shader:
ERROR: 0:5: 'sampler1D' : Reserved word. 
ERROR: 0:5: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
