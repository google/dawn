enable f16;

fn log_c9f489() {
  var arg_0 = 1.0h;
  var res : f16 = log(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f16;

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
