fn main_1() {
  let x_1 : bool = (30 > bitcast<i32>(20u));
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
