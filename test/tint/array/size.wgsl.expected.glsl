#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430) buffer s_block_ssbo {
  float inner;
} s;

void tint_symbol() {
  float signed_literal[4] = float[4](0.0f, 0.0f, 0.0f, 0.0f);
  float unsigned_literal[4] = float[4](0.0f, 0.0f, 0.0f, 0.0f);
  float signed_constant[4] = float[4](0.0f, 0.0f, 0.0f, 0.0f);
  float unsigned_constant[4] = float[4](0.0f, 0.0f, 0.0f, 0.0f);
  float shr_const_expr[4] = float[4](0.0f, 0.0f, 0.0f, 0.0f);
  unsigned_literal = signed_literal;
  signed_constant = signed_literal;
  unsigned_constant = signed_literal;
  shr_const_expr = signed_literal;
  s.inner = ((((signed_literal[0] + unsigned_literal[0]) + signed_constant[0]) + unsigned_constant[0]) + shr_const_expr[0]);
}

void main() {
  tint_symbol();
  return;
}
