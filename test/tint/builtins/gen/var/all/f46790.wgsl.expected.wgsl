fn all_f46790() {
  var arg_0 = vec2<bool>(true);
  var res : bool = all(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  all_f46790();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  all_f46790();
}

@compute @workgroup_size(1)
fn compute_main() {
  all_f46790();
}
