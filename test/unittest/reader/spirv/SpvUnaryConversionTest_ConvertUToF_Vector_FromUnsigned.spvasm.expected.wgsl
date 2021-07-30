fn main_1() {
  let x_30 : vec2<u32> = vec2<u32>(10u, 20u);
  let x_1 : vec2<f32> = vec2<f32>(x_30);
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
