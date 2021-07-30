fn main_1() {
  let x_1 : i32 = ~(30);
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
