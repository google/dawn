struct S {
  i : i32;
};

var<private> V : S;

[[stage(compute)]]
fn main() {
  var i : i32;
  let x_15 : i32 = V.i;
  i = x_15;
  return;
}
