enable f16;

fn log_c9f489() {
  var arg_0 = f16();
  var res : f16 = log(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  log_c9f489();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  log_c9f489();
}

@compute @workgroup_size(1)
fn compute_main() {
  log_c9f489();
}
