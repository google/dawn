struct S {
  i : i32;
};

[[stage(compute)]]
fn main() {
  var V : S;
  V.i = 5;
  return;
}
