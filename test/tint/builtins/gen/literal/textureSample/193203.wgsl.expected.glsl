#version 310 es
precision mediump float;

uniform highp sampler2DArray arg_0_arg_1;

void textureSample_193203() {
  vec4 res = textureOffset(arg_0_arg_1, vec3(vec2(1.0f), float(1u)), ivec2(1));
}

void fragment_main() {
  textureSample_193203();
}

void main() {
  fragment_main();
  return;
}
