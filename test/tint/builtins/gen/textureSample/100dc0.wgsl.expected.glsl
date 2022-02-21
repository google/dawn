#version 310 es
precision mediump float;

uniform highp sampler3D arg_0_arg_1;

void textureSample_100dc0() {
  vec4 res = textureOffset(arg_0_arg_1, vec3(0.0f, 0.0f, 0.0f), ivec3(0, 0, 0));
}

void fragment_main() {
  textureSample_100dc0();
}

void main() {
  fragment_main();
  return;
}
