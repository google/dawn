fn main_1() {
  let x_1 : vec2<bool> = (vec2<bool>(true, false) == vec2<bool>(false, true));
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
