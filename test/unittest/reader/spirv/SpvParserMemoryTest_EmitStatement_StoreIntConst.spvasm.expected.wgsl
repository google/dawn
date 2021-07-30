fn main_1() {
  var x_1 : i32;
  x_1 = 42;
  x_1 = 0;
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
