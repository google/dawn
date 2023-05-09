const x_22 = vec2<f32>(2.0f, 2.0f);

fn main_1() {
  let distance_1 : vec2<f32> = x_22;
  let x_10 : f32 = distance(distance_1, x_22);
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn main() {
  main_1();
}
