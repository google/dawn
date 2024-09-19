SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, rg32f) uniform highp writeonly image2D x_20;
void main_1() {
  imageStore(x_20, ivec2(1, 0), vec4(0.0f));
  return;
}

void tint_symbol() {
  main_1();
}

void main() {
  tint_symbol();
  return;
}
error: Error parsing GLSL shader:
ERROR: 0:5: 'image load-store format' : not supported with this profile: es
ERROR: 0:5: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
