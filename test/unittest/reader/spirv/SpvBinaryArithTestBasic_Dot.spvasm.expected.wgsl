fn main_1() {
  let x_1 : vec2<f32> = vec2<f32>(50.0, 60.0);
  let x_2 : vec2<f32> = vec2<f32>(60.0, 50.0);
  let x_3 : f32 = dot(x_1, x_2);
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
