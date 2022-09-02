#version 310 es
precision mediump float;

layout(location = 0) in float f_1;
layout(location = 1) flat in uint u_1;
struct S {
  float f;
  uint u;
  vec4 v;
};

layout(binding = 0, std430) buffer tint_symbol_block_ssbo {
  S inner;
} tint_symbol;

void frag_main(S tint_symbol_1) {
  float f = tint_symbol_1.f;
  uint u = tint_symbol_1.u;
  vec4 v = tint_symbol_1.v;
  tint_symbol.inner = tint_symbol_1;
}

void main() {
  S tint_symbol_2 = S(f_1, u_1, gl_FragCoord);
  frag_main(tint_symbol_2);
  return;
}
