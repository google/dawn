@compute @workgroup_size(1u, 1u, 1u)
fn main() {
  var m : mat3x3<f32> = mat3x3<f32>();
  _ = m[1i].y;
}
