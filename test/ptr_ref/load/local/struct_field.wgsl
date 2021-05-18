struct S {
  i : i32;
};

[[stage(compute)]]
fn main() {
  var V : S;
  var i : i32 = V.i;
  return;
}
