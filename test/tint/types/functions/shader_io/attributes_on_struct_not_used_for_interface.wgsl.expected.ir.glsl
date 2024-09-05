#version 310 es
precision highp float;
precision highp int;


struct S {
  float f;
  uint u;
  vec4 v;
};

layout(binding = 0, std430)
buffer tint_symbol_2_1_ssbo {
  S tint_symbol_1;
} v_1;
void tint_store_and_preserve_padding(inout S target, S value_param) {
  target.f = value_param.f;
  target.u = value_param.u;
  target.v = value_param.v;
}
void main() {
  tint_store_and_preserve_padding(v_1.tint_symbol_1, S(1.0f, 2u, vec4(3.0f)));
}
