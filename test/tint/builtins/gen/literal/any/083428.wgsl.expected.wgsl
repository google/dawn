fn any_083428() {
  var res : bool = any(vec4<bool>(true));
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
