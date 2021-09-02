let slen = 4;
let ulen = 4u;

[[stage(fragment)]]
fn main() {
  var signed_literal : array<f32, 4>;
  var unsigned_literal : array<f32, 4u>;
  var signed_constant : array<f32, slen>;
  var unsigned_constant : array<f32, ulen>;

  // Ensure that the types are compatible.
  signed_literal = unsigned_constant;
  signed_constant = unsigned_literal;
}
