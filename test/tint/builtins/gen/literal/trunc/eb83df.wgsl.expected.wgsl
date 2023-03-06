fn trunc_eb83df() {
  var res : f32 = trunc(1.5f);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

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
