fn main_1() {
  let x_1 : bool = !((50.0 == 60.0));
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
