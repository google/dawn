fn asin_7b6a44() {
  var res : vec2<f32> = asin(vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  asin_7b6a44();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  asin_7b6a44();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  asin_7b6a44();
}
