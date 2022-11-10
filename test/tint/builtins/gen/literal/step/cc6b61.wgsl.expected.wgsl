enable f16;

fn step_cc6b61() {
  var res : vec3<f16> = step(vec3<f16>(1.0h), vec3<f16>(1.0h));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  step_cc6b61();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  step_cc6b61();
}

@compute @workgroup_size(1)
fn compute_main() {
  step_cc6b61();
}
