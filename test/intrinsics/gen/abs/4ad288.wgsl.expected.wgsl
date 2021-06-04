fn abs_4ad288() {
  var res : i32 = abs(1);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  abs_4ad288();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  abs_4ad288();
}

[[stage(compute)]]
fn compute_main() {
  abs_4ad288();
}
