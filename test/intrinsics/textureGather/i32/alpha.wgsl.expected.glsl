#version 310 es
precision mediump float;

uniform highp isampler2D t;


void tint_symbol() {
  ivec4 res = textureGather(t, vec2(0.0f, 0.0f), 3);
  return;
}
void main() {
  tint_symbol();
}


