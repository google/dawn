SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;

int tint_ftoi(float v) {
  return ((v <= 2147483520.0f) ? ((v < -2147483648.0f) ? (-2147483647 - 1) : int(v)) : 2147483647);
}

uniform highp samplerCubeArray x_20_x_10;

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
  vec4 x_73 = texture(x_20_x_10, vec4(vf1234.xyz, float(tint_ftoi(round(vf1234.w)))));
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
error: Error parsing GLSL shader:
ERROR: 0:9: 'samplerCubeArray' : Reserved word. 
ERROR: 0:9: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
