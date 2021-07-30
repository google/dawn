fn main_1() {
  let x_1 : u32 = bitcast<u32>((30 >> 10u));
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
