fn main_1() {
  let x_14 : vec2<bool> = vec2<bool>(false, false);
  let x_11 : vec2<u32> = vec2<u32>(0u, 0u);
  let x_12 : vec2<i32> = vec2<i32>(0, 0);
  let x_13 : vec2<f32> = vec2<f32>(0.0, 0.0);
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
