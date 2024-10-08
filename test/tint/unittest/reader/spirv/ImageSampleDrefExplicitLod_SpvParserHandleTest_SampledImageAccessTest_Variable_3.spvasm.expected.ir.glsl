SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;

uniform highp sampler2DArrayShadow x_20_x_10;
int tint_f32_to_i32(float value) {
  return mix(2147483647, mix((-2147483647 - 1), int(value), (value >= -2147483648.0f)), (value <= 2147483520.0f));
}
void main_1() {
  float f1 = 1.0f;
  vec2 vf12 = vec2(1.0f, 2.0f);
  vec2 vf21 = vec2(2.0f, 1.0f);
  vec3 vf123 = vec3(1.0f, 2.0f, 3.0f);
  vec4 vf1234 = vec4(1.0f, 2.0f, 3.0f, 4.0f);
  int i1 = 1;
  ivec2 vi12 = ivec2(1, 2);
  ivec3 vi123 = ivec3(1, 2, 3);
  ivec4 vi1234 = ivec4(1, 2, 3, 4);
  uint u1 = 1u;
  uvec2 vu12 = uvec2(1u, 2u);
  uvec3 vu123 = uvec3(1u, 2u, 3u);
  uvec4 vu1234 = uvec4(1u, 2u, 3u, 4u);
  float coords1 = 1.0f;
  vec2 coords12 = vf12;
  vec4 coords1234 = vf1234;
  float x_79 = textureOffset(x_20_x_10, vec4(vf123.xy, float(tint_f32_to_i32(round(vf123[2u]))), 0.20000000298023223877f), ivec2(3, 4));
}
void main() {
  main_1();
}
error: Error parsing GLSL shader:
ERROR: 0:26: 'sampler' : TextureOffset does not support sampler2DArrayShadow :  ES Profile
ERROR: 0:26: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
