enable f16;

fn atanh_d2d8cd() {
  var res : f16 = atanh(f16());
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  atanh_d2d8cd();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  atanh_d2d8cd();
}

@compute @workgroup_size(1)
fn compute_main() {
  atanh_d2d8cd();
}
