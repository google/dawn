SKIP: FAILED

#version 310 es
precision mediump float;

struct S {
  float f;
  uint u;
  vec4 v;
};

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
ERROR: 0:26: 'assign' :  cannot convert from ' in structure{ global mediump float f,  global mediump uint u,  global mediump 4-component vector of float v}' to 'layout( binding=0 column_major shared) buffer block{layout( column_major shared) buffer mediump float f, layout( column_major shared) buffer mediump uint u, layout( column_major shared) buffer mediump 4-component vector of float v}'
ERROR: 0:26: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



