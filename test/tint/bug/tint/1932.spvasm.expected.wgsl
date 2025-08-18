@compute @workgroup_size(1u, 1u, 1u)
fn main() {
  let distance_1 = vec2<f32>(2.0f);
  _ = distance(distance_1, vec2<f32>(2.0f));
}
