enable f16;

fn step_cc6b61() {
  var arg_0 = vec3<f16>(1.0h);
  var arg_1 = vec3<f16>(1.0h);
  var res : vec3<f16> = step(arg_0, arg_1);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f16>;

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
