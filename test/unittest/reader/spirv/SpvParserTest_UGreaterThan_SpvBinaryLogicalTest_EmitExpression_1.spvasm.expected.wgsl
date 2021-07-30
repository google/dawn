fn main_1() {
  let x_1 : bool = (bitcast<u32>(30) > 20u);
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
