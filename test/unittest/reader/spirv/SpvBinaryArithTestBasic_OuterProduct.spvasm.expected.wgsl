fn main_1() {
  let x_1 : vec3<f32> = (vec3<f32>(50.0, 60.0, 70.0) + vec3<f32>(50.0, 60.0, 70.0));
  let x_2 : vec2<f32> = (vec2<f32>(60.0, 50.0) + vec2<f32>(50.0, 60.0));
  let x_3 : mat2x3<f32> = mat2x3<f32>(vec3<f32>((x_2.x * x_1.x), (x_2.x * x_1.y), (x_2.x * x_1.z)), vec3<f32>((x_2.y * x_1.x), (x_2.y * x_1.y), (x_2.y * x_1.z)));
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
