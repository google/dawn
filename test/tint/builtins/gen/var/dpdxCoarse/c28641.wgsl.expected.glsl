#version 310 es
precision mediump float;

void dpdxCoarse_c28641() {
  vec4 arg_0 = vec4(1.0f);
  vec4 res = dFdx(arg_0);
}

void fragment_main() {
  dpdxCoarse_c28641();
}

void main() {
  fragment_main();
  return;
}
