enable f16;

fn step_630d07() {
  var arg_0 = f16();
  var arg_1 = f16();
  var res : f16 = step(arg_0, arg_1);
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
