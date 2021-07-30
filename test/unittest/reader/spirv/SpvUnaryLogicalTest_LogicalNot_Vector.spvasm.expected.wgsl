fn main_1() {
  let x_1 : vec2<bool> = !(vec2<bool>(true, false));
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
