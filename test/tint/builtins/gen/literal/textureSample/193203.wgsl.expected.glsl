#version 310 es
precision mediump float;

uniform highp sampler2DArray arg_0_arg_1;

void textureSample_193203() {
  vec4 res = textureOffset(arg_0_arg_1, vec3(0.0f, 0.0f, float(1u)), ivec2(0));
}

void fragment_main() {
  textureSample_193203();
}

void main() {
  fragment_main();
  return;
}
