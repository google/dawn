fn main_1() {
  let x_1 : vec2<f32> = (vec2<f32>(50.0, 60.0) - (vec2<f32>(60.0, 50.0) * floor((vec2<f32>(50.0, 60.0) / vec2<f32>(60.0, 50.0)))));
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
