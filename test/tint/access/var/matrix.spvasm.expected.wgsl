fn main_1() {
  var m : mat3x3<f32> = mat3x3<f32>();
  let x_15 : vec3<f32> = m[1i];
  let x_16 : f32 = x_15.y;
  return;
}

@stage(compute) @workgroup_size(1, 1, 1)
fn main() {
  main_1();
}
