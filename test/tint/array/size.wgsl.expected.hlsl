static const int slen = 4;
static const uint ulen = 4u;

void main() {
  float signed_literal[4] = (float[4])0;
  float unsigned_literal[4] = (float[4])0;
  float signed_constant[4] = (float[4])0;
  float unsigned_constant[4] = (float[4])0;
  signed_literal = unsigned_constant;
  signed_constant = unsigned_literal;
  return;
}
