fn main_1() {
  let x_30 : i32 = 30;
  let x_1 : f32 = f32(bitcast<u32>(x_30));
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
