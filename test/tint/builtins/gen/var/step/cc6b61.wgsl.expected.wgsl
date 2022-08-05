enable f16;

fn step_cc6b61() {
  var arg_0 = vec3<f16>(f16());
  var arg_1 = vec3<f16>(f16());
  var res : vec3<f16> = step(arg_0, arg_1);
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
