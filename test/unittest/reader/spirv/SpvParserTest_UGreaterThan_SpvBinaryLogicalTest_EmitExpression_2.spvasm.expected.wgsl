fn main_1() {
  let x_1 : bool = (10u > bitcast<u32>(40));
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
