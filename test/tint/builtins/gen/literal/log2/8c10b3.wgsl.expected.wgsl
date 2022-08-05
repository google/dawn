enable f16;

fn log2_8c10b3() {
  var res : f16 = log2(f16());
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  log2_8c10b3();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  log2_8c10b3();
}

@compute @workgroup_size(1)
fn compute_main() {
  log2_8c10b3();
}
