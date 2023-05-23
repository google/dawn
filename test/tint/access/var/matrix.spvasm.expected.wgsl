fn main_1() {
  var m = mat3x3f();
  let x_15 = m[1i];
  let x_16 = x_15.y;
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn main() {
  main_1();
}
