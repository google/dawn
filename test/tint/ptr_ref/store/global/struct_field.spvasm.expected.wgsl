struct S {
  i : i32,
}

var<private> V : S;

fn main_1() {
  V.i = 5i;
  return;
}

@stage(compute) @workgroup_size(1, 1, 1)
fn main() {
  main_1();
}
