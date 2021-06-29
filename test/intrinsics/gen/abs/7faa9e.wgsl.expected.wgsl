fn abs_7faa9e() {
  var res : vec2<i32> = abs(vec2<i32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  abs_7faa9e();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  abs_7faa9e();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  abs_7faa9e();
}
