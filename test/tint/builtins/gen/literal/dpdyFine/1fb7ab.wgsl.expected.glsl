#version 310 es
precision mediump float;

void dpdyFine_1fb7ab() {
  vec3 res = dFdy(vec3(1.0f));
}

void fragment_main() {
  dpdyFine_1fb7ab();
}

void main() {
  fragment_main();
  return;
}
