#version 310 es
precision mediump float;

void dpdy_699a05() {
  vec4 arg_0 = vec4(1.0f);
  vec4 res = dFdy(arg_0);
}

void fragment_main() {
  dpdy_699a05();
}

void main() {
  fragment_main();
  return;
}
