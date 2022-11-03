fn ceil_e0b70a() {
  var res = ceil(1.5);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  ceil_e0b70a();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  ceil_e0b70a();
}

@compute @workgroup_size(1)
fn compute_main() {
  ceil_e0b70a();
}
