fn main_1() {
  let x_1 : i32 = (30 << bitcast<u32>(40));
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
