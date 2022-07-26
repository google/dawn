fn any_2ab91a() {
  var res : bool = any(true);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  any_2ab91a();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  any_2ab91a();
}

@compute @workgroup_size(1)
fn compute_main() {
  any_2ab91a();
}
