#version 310 es
precision mediump float;

uniform highp sampler2D arg_0;


void textureSample_667d76() {
  float res = textureOffset(arg_0, vec2(0.0f, 0.0f), ivec2(0, 0)).x;
}

void fragment_main() {
  textureSample_667d76();
  return;
}
void main() {
  fragment_main();
}


