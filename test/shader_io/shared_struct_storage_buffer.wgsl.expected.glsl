SKIP: FAILED

#version 310 es
precision mediump float;

layout(location = 0) in float f_1;
layout(location = 1) flat in uint u_1;
struct S {
  float f;
  uint u;
  vec4 v;
};

layout(binding = 0) buffer S_1 {
  float f;
  uint u;
  vec4 v;
} tint_symbol;
void frag_main(S tint_symbol_1) {
  float f = tint_symbol_1.f;
  uint u = tint_symbol_1.u;
  vec4 v = tint_symbol_1.v;
  tint_symbol = tint_symbol_1;
}

void main() {
  S tint_symbol_2 = S(f_1, u_1, gl_FragCoord);
  frag_main(tint_symbol_2);
  return;
}
Error parsing GLSL shader:
ERROR: 0:21: 'assign' :  cannot convert from ' in structure{ global mediump float f,  global mediump uint u,  global mediump 4-component vector of float v}' to 'layout( binding=0 column_major shared) buffer block{layout( column_major shared) buffer mediump float f, layout( column_major shared) buffer mediump uint u, layout( column_major shared) buffer mediump 4-component vector of float v}'
ERROR: 0:21: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



