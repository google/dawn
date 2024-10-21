#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  vec4 inner;
} v;
uniform highp sampler2D arg_0_arg_1;
vec4 textureSampleBias_a161cf() {
  vec2 arg_2 = vec2(1.0f);
  float arg_3 = 1.0f;
  vec2 v_1 = arg_2;
  vec4 res = textureOffset(arg_0_arg_1, v_1, ivec2(1), clamp(arg_3, -16.0f, 15.9899997711181640625f));
  return res;
}
void main() {
  v.inner = textureSampleBias_a161cf();
}
