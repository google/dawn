fn main_1() {
  var i : i32 = 0i;
  i = 123i;
  let x_10 : i32 = i;
  let x_12 : i32 = (x_10 + 1i);
  return;
}

@stage(compute) @workgroup_size(1, 1, 1)
fn main() {
  main_1();
}
