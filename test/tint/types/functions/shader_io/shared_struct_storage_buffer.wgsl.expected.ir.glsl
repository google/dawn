#version 310 es
precision highp float;
precision highp int;


struct S {
  float f;
  uint u;
  vec4 v;
};

layout(binding = 0, std430)
buffer tint_symbol_3_1_ssbo {
  S tint_symbol_2;
} v_1;
layout(location = 0) in float frag_main_loc0_Input;
layout(location = 1) flat in uint frag_main_loc1_Input;
void tint_store_and_preserve_padding(inout S target, S value_param) {
  target.f = value_param.f;
  target.u = value_param.u;
  target.v = value_param.v;
}
void frag_main_inner(S tint_symbol_1) {
  float f = tint_symbol_1.f;
  uint u = tint_symbol_1.u;
  vec4 v = tint_symbol_1.v;
  tint_store_and_preserve_padding(v_1.tint_symbol_2, tint_symbol_1);
}
void main() {
  frag_main_inner(S(frag_main_loc0_Input, frag_main_loc1_Input, gl_FragCoord));
}
