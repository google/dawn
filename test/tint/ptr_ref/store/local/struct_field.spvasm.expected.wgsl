struct S {
  i : i32,
}

fn main_1() {
  var V : S;
  V.i = 5i;
  return;
}

@stage(compute) @workgroup_size(1, 1, 1)
fn main() {
  main_1();
}
