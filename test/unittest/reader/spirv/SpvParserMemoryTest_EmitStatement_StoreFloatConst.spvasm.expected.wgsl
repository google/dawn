fn main_1() {
  var x_1 : f32;
  x_1 = 42.0;
  x_1 = 0.0;
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
