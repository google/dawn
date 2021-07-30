fn main_1() {
  let x_30 : u32 = 10u;
  let x_1 : f32 = f32(bitcast<i32>(x_30));
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
