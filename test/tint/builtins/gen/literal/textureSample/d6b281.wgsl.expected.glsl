#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

uniform highp sampler2DArray arg_0_arg_1;

vec4 textureSample_d6b281() {
  vec4 res = texture(arg_0_arg_1, vec3(vec2(1.0f), float(1u)));
  return res;
}

void fragment_main() {
  prevent_dce.inner = textureSample_d6b281();
}

void main() {
  fragment_main();
  return;
}
