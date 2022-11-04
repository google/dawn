#version 310 es
precision mediump float;

uniform highp sampler2DArray arg_0_arg_1;

void textureSample_193203() {
  vec2 arg_2 = vec2(1.0f);
  uint arg_3 = 1u;
  vec4 res = textureOffset(arg_0_arg_1, vec3(arg_2, float(arg_3)), ivec2(1));
}

void fragment_main() {
  textureSample_193203();
}

void main() {
  fragment_main();
  return;
}
