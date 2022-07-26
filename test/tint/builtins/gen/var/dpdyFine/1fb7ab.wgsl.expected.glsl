#version 310 es
precision mediump float;

void dpdyFine_1fb7ab() {
  vec3 arg_0 = vec3(1.0f);
  vec3 res = dFdy(arg_0);
}

void fragment_main() {
  dpdyFine_1fb7ab();
}

void main() {
  fragment_main();
  return;
}
