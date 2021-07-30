fn main_1() {
  let x_1 : bool = select(false, true, true);
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
