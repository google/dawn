SKIP: FAILED

#version 310 es
precision mediump float;


layout (binding = 0) buffer S_1 {
  float f;
  uint u;
  vec4 v;
} tint_symbol;

struct tint_symbol_3 {
  float f;
  uint u;
  vec4 v;
};

void frag_main_inner(S tint_symbol_1) {
  float f = tint_symbol_1.f;
  uint u = tint_symbol_1.u;
  vec4 v = tint_symbol_1.v;
  tint_symbol = tint_symbol_1;
}

void frag_main(tint_symbol_3 tint_symbol_2) {
  S tint_symbol_4 = S(tint_symbol_2.f, tint_symbol_2.u, tint_symbol_2.v);
  frag_main_inner(tint_symbol_4);
  return;
}
in float f;
in uint u;
void main() {
  tint_symbol_3 inputs;
  inputs.f = f;
  inputs.u = u;
  inputs.v = gl_FragCoord;
  frag_main(inputs);
}


Error parsing GLSL shader:
ERROR: 0:17: '' :  syntax error, unexpected IDENTIFIER, expecting RIGHT_PAREN
ERROR: 1 compilation errors.  No code generated.



