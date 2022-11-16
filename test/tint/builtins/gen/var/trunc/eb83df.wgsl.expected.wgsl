fn trunc_eb83df() {
  var arg_0 = 1.5f;
  var res : f32 = trunc(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  trunc_eb83df();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  trunc_eb83df();
}

@compute @workgroup_size(1)
fn compute_main() {
  trunc_eb83df();
}
