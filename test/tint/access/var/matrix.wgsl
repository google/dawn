@compute @workgroup_size(1)
fn main() {
  var m : mat3x3<f32>;
  let v : vec3<f32> = m[1];
  let f : f32 = v[1];
}
