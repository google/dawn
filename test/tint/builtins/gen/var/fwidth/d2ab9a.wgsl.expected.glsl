#version 310 es
precision mediump float;

void fwidth_d2ab9a() {
  vec4 arg_0 = vec4(1.0f);
  vec4 res = fwidth(arg_0);
}

void fragment_main() {
  fwidth_d2ab9a();
}

void main() {
  fragment_main();
  return;
}
