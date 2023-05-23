fn main_1() {
  var a : f32;
  var b : f32;
  a = 42.0f;
  let x_11 = a;
  b = degrees(x_11);
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn main() {
  main_1();
}
