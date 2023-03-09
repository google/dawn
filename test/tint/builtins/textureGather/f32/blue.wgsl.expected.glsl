#version 310 es
precision highp float;

uniform highp sampler2D t_s;

void tint_symbol() {
  vec4 res = textureGather(t_s, vec2(0.0f), 2);
}

void main() {
  tint_symbol();
  return;
}
