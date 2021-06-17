struct S {
  i : i32;
};

var<private> V : S;

fn main_1() {
  V.i = 5;
  return;
}

[[stage(compute)]]
fn main() {
  main_1();
}
