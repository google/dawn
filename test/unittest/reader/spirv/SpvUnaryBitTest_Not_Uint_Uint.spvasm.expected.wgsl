fn main_1() {
  let x_1 : u32 = ~(10u);
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
