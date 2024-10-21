#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  vec4 inner;
} v;
uniform highp sampler2DArray arg_0_arg_1;
vec4 textureSampleBias_87915c() {
  vec2 arg_2 = vec2(1.0f);
  uint arg_3 = 1u;
  float arg_4 = 1.0f;
  vec2 v_1 = arg_2;
  uint v_2 = arg_3;
  float v_3 = clamp(arg_4, -16.0f, 15.9899997711181640625f);
  vec4 res = textureOffset(arg_0_arg_1, vec3(v_1, float(v_2)), ivec2(1), v_3);
  return res;
}
void main() {
  v.inner = textureSampleBias_87915c();
}
