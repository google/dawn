#version 460
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  vec4 inner;
} v;
uniform highp samplerCubeArray f_arg_0_arg_1;
vec4 textureSampleBias_eed7c4() {
  float v_1 = float(1);
  vec4 res = texture(f_arg_0_arg_1, vec4(1.0f), 1.0f);
  return res;
}
void main() {
  v.inner = textureSampleBias_eed7c4();
}
