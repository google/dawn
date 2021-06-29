fn asin_064953() {
  var res : vec4<f32> = asin(vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  asin_064953();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  asin_064953();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  asin_064953();
}
