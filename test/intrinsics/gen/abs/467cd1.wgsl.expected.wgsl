fn abs_467cd1() {
  var res : u32 = abs(1u);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  abs_467cd1();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  abs_467cd1();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  abs_467cd1();
}
