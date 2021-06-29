fn abs_7f28e6() {
  var res : vec2<u32> = abs(vec2<u32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  abs_7f28e6();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  abs_7f28e6();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  abs_7f28e6();
}
