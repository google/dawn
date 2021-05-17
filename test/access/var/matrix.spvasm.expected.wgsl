[[stage(compute)]]
fn main() {
  var m : mat3x3<f32> = mat3x3<f32>(vec3<f32>(0.0, 0.0, 0.0), vec3<f32>(0.0, 0.0, 0.0), vec3<f32>(0.0, 0.0, 0.0));
  let x_15 : vec3<f32> = m[1];
  let x_16 : f32 = x_15.y;
  return;
}
