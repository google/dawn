#version 310 es
precision mediump float;

void dpdyFine_d0a648() {
  vec4 arg_0 = vec4(1.0f);
  vec4 res = dFdy(arg_0);
}

void fragment_main() {
  dpdyFine_d0a648();
}

void main() {
  fragment_main();
  return;
}
