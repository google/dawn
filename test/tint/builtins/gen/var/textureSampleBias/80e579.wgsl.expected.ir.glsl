#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  vec4 inner;
} v;
uniform highp sampler2DArray arg_0_arg_1;
vec4 textureSampleBias_80e579() {
  vec2 arg_2 = vec2(1.0f);
  int arg_3 = 1;
  float arg_4 = 1.0f;
  vec2 v_1 = arg_2;
  int v_2 = arg_3;
  float v_3 = clamp(arg_4, -16.0f, 15.9899997711181640625f);
  vec4 res = texture(arg_0_arg_1, vec3(v_1, float(v_2)), v_3);
  return res;
}
void main() {
  v.inner = textureSampleBias_80e579();
}
