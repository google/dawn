struct S {
  i : i32;
};

fn main_1() {
  var V : S;
  V.i = 5;
  return;
}

[[stage(compute)]]
fn main() {
  main_1();
}
