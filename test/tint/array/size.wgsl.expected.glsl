#version 310 es
precision highp float;

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
}

void main() {
  tint_symbol();
  return;
}
