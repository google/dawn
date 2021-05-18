[[stage(compute)]]
fn main() {
  var v : vec3<f32> = vec3<f32>(0.0, 0.0, 0.0);
  v = vec3<f32>(1.0, 2.0, 3.0);
  v.y = 5.0;
  return;
}
