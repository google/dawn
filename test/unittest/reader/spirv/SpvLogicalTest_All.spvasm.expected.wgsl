fn main_1() {
  let x_1 : bool = all(vec2<bool>(true, false));
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
