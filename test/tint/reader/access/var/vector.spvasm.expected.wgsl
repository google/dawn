@compute @workgroup_size(1u, 1u, 1u)
fn main() {
  var v : vec3<f32> = vec3<f32>();
  _ = v.y;
  _ = v.xz;
  _ = v.xzy;
}
