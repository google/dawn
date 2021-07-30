fn main_1() {
  let x_1 : u32 = bitcast<u32>((bitcast<i32>(10u) >> 20u));
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
