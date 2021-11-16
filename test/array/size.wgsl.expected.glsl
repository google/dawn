#version 310 es
precision mediump float;

const int slen = 4;
const uint ulen = 4u;

void tint_symbol() {
  float signed_literal[4] = float[4](0.0f, 0.0f, 0.0f, 0.0f);
  float unsigned_literal[4] = float[4](0.0f, 0.0f, 0.0f, 0.0f);
  float signed_constant[4] = float[4](0.0f, 0.0f, 0.0f, 0.0f);
  float unsigned_constant[4] = float[4](0.0f, 0.0f, 0.0f, 0.0f);
  signed_literal = unsigned_constant;
  signed_constant = unsigned_literal;
  return;
}
void main() {
  tint_symbol();
}


