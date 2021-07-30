fn main_1() {
  let x_1 : i32 = (bitcast<i32>(10u) / 30);
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
