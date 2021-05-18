struct S {
  i : i32;
};

[[stage(compute)]]
fn main() {
  var i : i32;
  var V : S;
  let x_14 : i32 = V.i;
  i = x_14;
  return;
}
