fn main_1() {
  var x_1 : u32;
  x_1 = 42u;
  x_1 = 0u;
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
