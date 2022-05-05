fn main_1() {
  var a : f32;
  var b : f32;
  a = 42.0;
  let x_11 : f32 = a;
  b = radians(x_11);
  return;
}

@stage(compute) @workgroup_size(1i, 1i, 1i)
fn main() {
  main_1();
}
