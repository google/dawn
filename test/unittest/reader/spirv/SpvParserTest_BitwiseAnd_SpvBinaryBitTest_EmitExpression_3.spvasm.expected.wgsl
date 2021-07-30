fn main_1() {
  let x_1 : vec2<i32> = (vec2<i32>(30, 40) & vec2<i32>(40, 30));
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
