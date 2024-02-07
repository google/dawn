#version 310 es
precision highp float;
precision highp int;

ivec4 g() {
  return ivec4(0);
}

void tint_symbol() {
  g();
}

void main() {
  tint_symbol();
  return;
}
