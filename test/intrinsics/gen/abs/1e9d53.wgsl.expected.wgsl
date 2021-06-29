fn abs_1e9d53() {
  var res : vec2<f32> = abs(vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  abs_1e9d53();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  abs_1e9d53();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  abs_1e9d53();
}
