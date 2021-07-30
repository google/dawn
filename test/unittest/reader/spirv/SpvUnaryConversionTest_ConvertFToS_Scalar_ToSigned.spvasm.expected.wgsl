fn main_1() {
  let x_30 : f32 = 50.0;
  let x_1 : i32 = i32(x_30);
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
