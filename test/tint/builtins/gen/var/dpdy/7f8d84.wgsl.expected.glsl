#version 310 es
precision mediump float;

void dpdy_7f8d84() {
  float arg_0 = 1.0f;
  float res = dFdy(arg_0);
}

void fragment_main() {
  dpdy_7f8d84();
}

void main() {
  fragment_main();
  return;
}
