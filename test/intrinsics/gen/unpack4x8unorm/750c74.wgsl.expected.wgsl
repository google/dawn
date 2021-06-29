fn unpack4x8unorm_750c74() {
  var res : vec4<f32> = unpack4x8unorm(1u);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  unpack4x8unorm_750c74();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  unpack4x8unorm_750c74();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  unpack4x8unorm_750c74();
}
