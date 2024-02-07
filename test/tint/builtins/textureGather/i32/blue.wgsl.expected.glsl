#version 310 es
precision highp float;
precision highp int;

uniform highp isampler2D t_s;

void tint_symbol() {
  ivec4 res = textureGather(t_s, vec2(0.0f), 2);
}

void main() {
  tint_symbol();
  return;
}
