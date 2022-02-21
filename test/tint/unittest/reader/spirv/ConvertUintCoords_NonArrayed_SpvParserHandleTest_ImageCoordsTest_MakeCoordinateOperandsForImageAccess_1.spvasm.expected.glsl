SKIP: FAILED

#version 310 es
precision mediump float;

uniform highp sampler1D x_20_1;
void main_1() {
  float float_var = 0.0f;
  int i1 = 1;
  ivec2 vi12 = ivec2(1, 2);
  ivec3 vi123 = ivec3(1, 2, 3);
  ivec4 vi1234 = ivec4(1, 2, 3, 4);
  uint u1 = 1u;
  uvec2 vu12 = uvec2(1u, 2u);
  uvec3 vu123 = uvec3(1u, 2u, 3u);
  uvec4 vu1234 = uvec4(1u, 2u, 3u, 4u);
  float f1 = 1.0f;
  vec2 vf12 = vec2(1.0f, 2.0f);
  vec3 vf123 = vec3(1.0f, 2.0f, 3.0f);
  vec4 vf1234 = vec4(1.0f, 2.0f, 3.0f, 4.0f);
  vec4 x_71 = texelFetch(x_20_1, int(u1), 0);
  uint x_1000 = 0u;
  return;
}

void tint_symbol() {
  main_1();
}

void main() {
  tint_symbol();
  return;
}
Error parsing GLSL shader:
ERROR: 0:4: 'sampler1D' : Reserved word. 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



