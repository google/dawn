fn asin_c0c272() {
  var res : f32 = asin(1.0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  asin_c0c272();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  asin_c0c272();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  asin_c0c272();
}
