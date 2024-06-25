#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

uniform highp sampler3D arg_0_arg_1;

vec4 textureSampleBias_594824() {
  vec3 arg_2 = vec3(1.0f);
  float arg_3 = 1.0f;
  vec4 res = textureOffset(arg_0_arg_1, arg_2, ivec3(1), arg_3);
  return res;
}

void fragment_main() {
  prevent_dce.inner = textureSampleBias_594824();
}

void main() {
  fragment_main();
  return;
}
