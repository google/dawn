fn main_1() {
  let x_30 : vec2<i32> = vec2<i32>(30, 40);
  let x_1 : vec2<f32> = vec2<f32>(x_30);
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
