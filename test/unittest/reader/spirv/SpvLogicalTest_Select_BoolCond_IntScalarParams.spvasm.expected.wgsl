fn main_1() {
  let x_1 : u32 = select(20u, 10u, true);
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
