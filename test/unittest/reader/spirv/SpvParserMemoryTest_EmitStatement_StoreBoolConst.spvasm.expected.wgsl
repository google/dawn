fn main_1() {
  var x_1 : bool;
  x_1 = true;
  x_1 = false;
  x_1 = false;
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
