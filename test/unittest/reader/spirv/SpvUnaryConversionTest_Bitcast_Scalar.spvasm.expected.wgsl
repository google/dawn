fn main_1() {
  let x_1 : u32 = bitcast<u32>(50.0);
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
