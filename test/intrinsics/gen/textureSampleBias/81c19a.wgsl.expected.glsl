#version 310 es
precision mediump float;

uniform highp sampler2D arg_0;


void textureSampleBias_81c19a() {
  vec4 res = textureOffset(arg_0, vec2(0.0f, 0.0f), ivec2(0, 0), 1.0f);
}

void fragment_main() {
  textureSampleBias_81c19a();
  return;
}
void main() {
  fragment_main();
}


