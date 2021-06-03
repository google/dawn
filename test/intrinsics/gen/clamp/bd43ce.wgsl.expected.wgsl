fn clamp_bd43ce() {
  var res : vec4<u32> = clamp(vec4<u32>(), vec4<u32>(), vec4<u32>());
}

[[stage(vertex)]]
fn vertex_main() {
  clamp_bd43ce();
}

[[stage(fragment)]]
fn fragment_main() {
  clamp_bd43ce();
}

[[stage(compute)]]
fn compute_main() {
  clamp_bd43ce();
}
