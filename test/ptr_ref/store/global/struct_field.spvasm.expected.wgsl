struct S {
  i : i32;
};

var<private> V : S;

[[stage(compute)]]
fn main() {
  V.i = 5;
  return;
}
