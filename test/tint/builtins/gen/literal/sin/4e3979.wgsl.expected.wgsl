fn sin_4e3979() {
  var res : vec4<f32> = sin(vec4<f32>(1.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sin_4e3979();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sin_4e3979();
}

@compute @workgroup_size(1)
fn compute_main() {
  sin_4e3979();
}
