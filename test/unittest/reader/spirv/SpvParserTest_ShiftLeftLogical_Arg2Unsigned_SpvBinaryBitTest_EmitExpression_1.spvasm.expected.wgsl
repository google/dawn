fn main_1() {
  let x_1 : i32 = (30 << 20u);
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
