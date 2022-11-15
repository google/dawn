fn max_19070a() {
  var res = max(vec4(1), vec4(1));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  max_19070a();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  max_19070a();
}

@compute @workgroup_size(1)
fn compute_main() {
  max_19070a();
}
