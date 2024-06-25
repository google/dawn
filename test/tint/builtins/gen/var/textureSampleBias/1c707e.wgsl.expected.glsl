#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

uniform highp sampler2DArray arg_0_arg_1;

vec4 textureSampleBias_1c707e() {
  vec2 arg_2 = vec2(1.0f);
  uint arg_3 = 1u;
  float arg_4 = 1.0f;
  vec4 res = texture(arg_0_arg_1, vec3(arg_2, float(arg_3)), arg_4);
  return res;
}

void fragment_main() {
  prevent_dce.inner = textureSampleBias_1c707e();
}

void main() {
  fragment_main();
  return;
}
