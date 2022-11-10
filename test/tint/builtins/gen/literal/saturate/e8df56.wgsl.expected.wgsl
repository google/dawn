enable f16;

fn saturate_e8df56() {
  var res : f16 = saturate(2.0h);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  saturate_e8df56();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  saturate_e8df56();
}

@compute @workgroup_size(1)
fn compute_main() {
  saturate_e8df56();
}
