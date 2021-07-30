fn main_1() {
  let x_1 : u32 = (10u << 20u);
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
