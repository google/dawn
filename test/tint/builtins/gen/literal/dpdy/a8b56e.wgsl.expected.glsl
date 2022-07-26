#version 310 es
precision mediump float;

void dpdy_a8b56e() {
  vec2 res = dFdy(vec2(1.0f));
}

void fragment_main() {
  dpdy_a8b56e();
}

void main() {
  fragment_main();
  return;
}
