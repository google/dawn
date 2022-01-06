#version 310 es
precision mediump float;

uniform highp usampler2D t;


void tint_symbol() {
  uvec4 res = textureGather(t, vec2(0.0f, 0.0f), 2);
  return;
}
void main() {
  tint_symbol();
}


