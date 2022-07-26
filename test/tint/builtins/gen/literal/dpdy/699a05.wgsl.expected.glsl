#version 310 es
precision mediump float;

void dpdy_699a05() {
  vec4 res = dFdy(vec4(1.0f));
}

void fragment_main() {
  dpdy_699a05();
}

void main() {
  fragment_main();
  return;
}
