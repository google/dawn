struct S {
  i : i32;
};

fn main_1() {
  var i : i32;
  var V : S;
  let x_14 : i32 = V.i;
  i = x_14;
  return;
}

[[stage(compute), workgroup_size(1, 1, 1)]]
fn main() {
  main_1();
}
