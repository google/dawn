fn main_1() {
  let x_1 : vec2<bool> = isInf(vec2<f32>(50.0, 60.0));
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
