#version 310 es
precision mediump float;

void dpdy_a8b56e() {
  vec2 arg_0 = vec2(1.0f);
  vec2 res = dFdy(arg_0);
}

void fragment_main() {
  dpdy_a8b56e();
}

void main() {
  fragment_main();
  return;
}
