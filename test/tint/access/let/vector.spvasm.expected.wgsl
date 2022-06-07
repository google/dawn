fn main_1() {
  let x_11 : f32 = vec3<f32>(1.0f, 2.0f, 3.0f).y;
  let x_13 : vec2<f32> = vec2<f32>(vec3<f32>(1.0f, 2.0f, 3.0f).x, vec3<f32>(1.0f, 2.0f, 3.0f).z);
  let x_14 : vec3<f32> = vec3<f32>(vec3<f32>(1.0f, 2.0f, 3.0f).x, vec3<f32>(1.0f, 2.0f, 3.0f).z, vec3<f32>(1.0f, 2.0f, 3.0f).y);
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn main() {
  main_1();
}
