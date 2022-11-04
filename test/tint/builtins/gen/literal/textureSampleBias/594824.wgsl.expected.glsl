#version 310 es
precision mediump float;

uniform highp sampler3D arg_0_arg_1;

void textureSampleBias_594824() {
  vec4 res = textureOffset(arg_0_arg_1, vec3(1.0f), ivec3(1), 1.0f);
}

void fragment_main() {
  textureSampleBias_594824();
}

void main() {
  fragment_main();
  return;
}
