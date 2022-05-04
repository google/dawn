fn main_1() {
  var m : mat3x3<f32> = mat3x3<f32>();
  m = mat3x3<f32>(vec3<f32>(1.0, 2.0, 3.0), vec3<f32>(4.0, 5.0, 6.0), vec3<f32>(7.0, 8.0, 9.0));
  m[1i] = vec3<f32>(5.0, 5.0, 5.0);
  return;
}

@stage(compute) @workgroup_size(1, 1, 1)
fn main() {
  main_1();
}
