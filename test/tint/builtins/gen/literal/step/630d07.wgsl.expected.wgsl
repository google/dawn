enable f16;

fn step_630d07() {
  var res : f16 = step(f16(), f16());
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  step_630d07();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  step_630d07();
}

@compute @workgroup_size(1)
fn compute_main() {
  step_630d07();
}
