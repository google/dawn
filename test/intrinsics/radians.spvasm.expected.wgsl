fn main_1() {
  var a : f32;
  var b : f32;
  a = 42.0;
  let x_11 : f32 = a;
  b = (x_11 * 0.017453292);
  return;
}

[[stage(compute), workgroup_size(1, 1, 1)]]
fn main() {
  main_1();
}
