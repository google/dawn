enable f16;

fn min_ac84d6() {
  var res : f16 = min(f16(), f16());
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  min_ac84d6();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  min_ac84d6();
}

@compute @workgroup_size(1)
fn compute_main() {
  min_ac84d6();
}
