fn trunc_f370d3() {
  var res : vec2<f32> = trunc(vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  trunc_f370d3();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  trunc_f370d3();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  trunc_f370d3();
}
