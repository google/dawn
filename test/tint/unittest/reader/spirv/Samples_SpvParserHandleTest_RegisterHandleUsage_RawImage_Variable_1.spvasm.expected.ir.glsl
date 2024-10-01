SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, rg32f) uniform highp writeonly image1D x_20;
void main_1() {
  imageStore(x_20, 1, vec4(0.0f));
}
void main() {
  main_1();
}
error: Error parsing GLSL shader:
ERROR: 0:5: 'image load-store format' : not supported with this profile: es
ERROR: 0:5: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
