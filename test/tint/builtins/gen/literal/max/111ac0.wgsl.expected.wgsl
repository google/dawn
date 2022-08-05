enable f16;

fn max_111ac0() {
  var res : f16 = max(f16(), f16());
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  max_111ac0();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  max_111ac0();
}

@compute @workgroup_size(1)
fn compute_main() {
  max_111ac0();
}
