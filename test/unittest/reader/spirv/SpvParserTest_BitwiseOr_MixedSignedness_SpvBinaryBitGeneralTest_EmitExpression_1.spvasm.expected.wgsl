fn main_1() {
  let x_1 : i32 = (30 | bitcast<i32>(10u));
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
