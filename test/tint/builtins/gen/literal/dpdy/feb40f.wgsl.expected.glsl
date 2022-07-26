#version 310 es
precision mediump float;

void dpdy_feb40f() {
  vec3 res = dFdy(vec3(1.0f));
}

void fragment_main() {
  dpdy_feb40f();
}

void main() {
  fragment_main();
  return;
}
