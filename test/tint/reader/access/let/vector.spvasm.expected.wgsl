@compute @workgroup_size(1u, 1u, 1u)
fn main() {
  _ = vec3<f32>(1.0f, 2.0f, 3.0f).y;
  _ = vec3<f32>(1.0f, 2.0f, 3.0f).xz;
  _ = vec3<f32>(1.0f, 2.0f, 3.0f).xzy;
}
