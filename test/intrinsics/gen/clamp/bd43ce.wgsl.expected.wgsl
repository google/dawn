fn clamp_bd43ce() {
  var res : vec4<u32> = clamp(vec4<u32>(), vec4<u32>(), vec4<u32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  clamp_bd43ce();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  clamp_bd43ce();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  clamp_bd43ce();
}
