fn ceil_b74c16() {
  var res : vec4<f32> = ceil(vec4<f32>(1.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  ceil_b74c16();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  ceil_b74c16();
}

@compute @workgroup_size(1)
fn compute_main() {
  ceil_b74c16();
}
