fn any_0e3e58() {
  var arg_0 = vec2<bool>(true);
  var res : bool = any(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  any_0e3e58();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  any_0e3e58();
}

@compute @workgroup_size(1)
fn compute_main() {
  any_0e3e58();
}
