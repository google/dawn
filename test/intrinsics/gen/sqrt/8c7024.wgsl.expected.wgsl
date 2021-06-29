fn sqrt_8c7024() {
  var res : vec2<f32> = sqrt(vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  sqrt_8c7024();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  sqrt_8c7024();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  sqrt_8c7024();
}
