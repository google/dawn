fn any_083428() {
  var arg_0 = vec4<bool>(true);
  var res : bool = any(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  any_083428();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  any_083428();
}

@compute @workgroup_size(1)
fn compute_main() {
  any_083428();
}
