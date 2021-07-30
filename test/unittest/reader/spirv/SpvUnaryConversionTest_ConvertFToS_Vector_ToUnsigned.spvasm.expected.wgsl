fn main_1() {
  let x_30 : vec2<f32> = vec2<f32>(50.0, 60.0);
  let x_1 : vec2<u32> = bitcast<vec2<u32>>(vec2<i32>(x_30));
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
