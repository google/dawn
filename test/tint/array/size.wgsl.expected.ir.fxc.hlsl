
RWByteAddressBuffer s : register(u0);
void main() {
  float signed_literal[4] = (float[4])0;
  float unsigned_literal[4] = (float[4])0;
  float signed_constant[4] = (float[4])0;
  float unsigned_constant[4] = (float[4])0;
  float shr_const_expr[4] = (float[4])0;
  float v[4] = signed_literal;
  unsigned_literal = v;
  float v_1[4] = signed_literal;
  signed_constant = v_1;
  float v_2[4] = signed_literal;
  unsigned_constant = v_2;
  float v_3[4] = signed_literal;
  shr_const_expr = v_3;
  s.Store(0u, asuint(((((signed_literal[0] + unsigned_literal[0]) + signed_constant[0]) + unsigned_constant[0]) + shr_const_expr[0])));
}

