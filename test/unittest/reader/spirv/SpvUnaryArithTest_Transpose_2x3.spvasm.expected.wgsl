fn main_1() {
  let x_1 : mat2x3<f32> = mat2x3<f32>(vec3<f32>(50.0, 60.0, 70.0), vec3<f32>(60.0, 70.0, 50.0));
  let x_2 : mat3x2<f32> = transpose(x_1);
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
