enable f16;

fn round_9078ef() {
  var arg_0 = 3.3984375h;
  var res : f16 = round(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  round_9078ef();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  round_9078ef();
}

@compute @workgroup_size(1)
fn compute_main() {
  round_9078ef();
}
