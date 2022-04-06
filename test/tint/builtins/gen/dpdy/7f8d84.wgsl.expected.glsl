#version 310 es
precision mediump float;

void dpdy_7f8d84() {
  float res = dFdy(1.0f);
}

void fragment_main() {
  dpdy_7f8d84();
}

void main() {
  fragment_main();
  return;
}
