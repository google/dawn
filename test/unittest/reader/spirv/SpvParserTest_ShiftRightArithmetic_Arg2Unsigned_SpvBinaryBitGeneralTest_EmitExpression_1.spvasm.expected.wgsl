fn main_1() {
  let x_1 : i32 = (30 >> 10u);
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
