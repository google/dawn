#version 310 es
precision mediump float;

void dpdy_feb40f() {
  vec3 arg_0 = vec3(1.0f);
  vec3 res = dFdy(arg_0);
}

void fragment_main() {
  dpdy_feb40f();
}

void main() {
  fragment_main();
  return;
}
