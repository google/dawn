#version 460
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  vec4 inner;
} v;
uniform highp samplerCubeArray arg_0_arg_1;
vec4 textureSampleBias_c6953d() {
  vec3 arg_2 = vec3(1.0f);
  uint arg_3 = 1u;
  float arg_4 = 1.0f;
  vec3 v_1 = arg_2;
  uint v_2 = arg_3;
  float v_3 = clamp(arg_4, -16.0f, 15.9899997711181640625f);
  vec4 res = texture(arg_0_arg_1, vec4(v_1, float(v_2)), v_3);
  return res;
}
void main() {
  v.inner = textureSampleBias_c6953d();
}
