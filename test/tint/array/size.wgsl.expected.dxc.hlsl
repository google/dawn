void main() {
  float signed_literal[4] = (float[4])0;
  float unsigned_literal[4] = (float[4])0;
  float signed_constant[4] = (float[4])0;
  float unsigned_constant[4] = (float[4])0;
  float shr_const_expr[4] = (float[4])0;
  unsigned_literal = signed_literal;
  signed_constant = signed_literal;
  unsigned_constant = signed_literal;
  shr_const_expr = signed_literal;
  return;
}
