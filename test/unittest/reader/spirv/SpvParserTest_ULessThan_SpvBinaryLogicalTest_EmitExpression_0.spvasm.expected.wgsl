fn main_1() {
  let x_1 : bool = (10u < 20u);
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
