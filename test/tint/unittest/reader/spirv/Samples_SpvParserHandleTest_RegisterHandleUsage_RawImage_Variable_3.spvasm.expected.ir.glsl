SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;

uniform highp sampler2D x_20;
void main_1() {
  highp sampler2D v = x_20;
  uvec2 x_125 = uvec2(textureSize(v, int(1u)));
}
void main() {
  main_1();
}
error: Error parsing GLSL shader:
ERROR: 0:7: 'sampler2D' : sampler/image types can only be used in uniform variables or function parameters: v
ERROR: 0:7: '=' :  cannot convert from ' uniform highp sampler2D' to ' temp highp sampler2D'
ERROR: 0:7: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
