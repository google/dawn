fn main_1() {
  let x_11 : f32 = vec3<f32>(1.0, 2.0, 3.0).y;
  let x_13 : vec2<f32> = vec2<f32>(vec3<f32>(1.0, 2.0, 3.0).x, vec3<f32>(1.0, 2.0, 3.0).z);
  let x_14 : vec3<f32> = vec3<f32>(vec3<f32>(1.0, 2.0, 3.0).x, vec3<f32>(1.0, 2.0, 3.0).z, vec3<f32>(1.0, 2.0, 3.0).y);
  return;
}

@stage(compute) @workgroup_size(1i, 1i, 1i)
fn main() {
  main_1();
}
