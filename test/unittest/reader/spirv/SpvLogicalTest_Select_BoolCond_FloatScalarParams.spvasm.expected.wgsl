fn main_1() {
  let x_1 : f32 = select(60.0, 50.0, true);
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
