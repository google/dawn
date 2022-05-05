struct S {
  i : i32,
}

var<private> V : S;

fn main_1() {
  var i : i32;
  let x_15 : i32 = V.i;
  i = x_15;
  return;
}

@stage(compute) @workgroup_size(1i, 1i, 1i)
fn main() {
  main_1();
}
