fn main_1() {
  let x_1 : i32 = bitcast<i32>((bitcast<u32>(30) >> 20u));
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
