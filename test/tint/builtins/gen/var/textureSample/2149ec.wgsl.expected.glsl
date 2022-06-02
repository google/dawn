#version 310 es
precision mediump float;

uniform highp sampler3D arg_0_arg_1;

void textureSample_2149ec() {
  vec3 arg_2 = vec3(0.0f);
  vec4 res = textureOffset(arg_0_arg_1, arg_2, ivec3(0));
}

void fragment_main() {
  textureSample_2149ec();
}

void main() {
  fragment_main();
  return;
}
