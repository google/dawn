enable f16;

fn normalize_b8cb8d() {
  var res : vec4<f16> = normalize(vec4<f16>(1.0h));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  normalize_b8cb8d();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  normalize_b8cb8d();
}

@compute @workgroup_size(1)
fn compute_main() {
  normalize_b8cb8d();
}
