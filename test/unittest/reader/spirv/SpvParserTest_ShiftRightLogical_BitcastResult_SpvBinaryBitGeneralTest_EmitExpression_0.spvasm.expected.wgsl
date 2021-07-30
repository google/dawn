fn main_1() {
  let x_1 : i32 = bitcast<i32>((20u >> 10u));
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
