fn main_1() {
  let x_11 : mat2x2<f32> = mat2x2<f32>(vec2<f32>(0.0, 0.0), vec2<f32>(0.0, 0.0));
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
