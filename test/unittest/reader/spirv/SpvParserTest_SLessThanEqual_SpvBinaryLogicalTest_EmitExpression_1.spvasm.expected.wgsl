fn main_1() {
  let x_1 : bool = (bitcast<i32>(10u) <= 40);
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
