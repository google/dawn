#version 310 es
precision mediump float;

uniform highp sampler2D arg_0;


void textureSample_38bbb9() {
  float res = texture(arg_0, vec2(0.0f, 0.0f)).x;
}

void fragment_main() {
  textureSample_38bbb9();
  return;
}
void main() {
  fragment_main();
}


