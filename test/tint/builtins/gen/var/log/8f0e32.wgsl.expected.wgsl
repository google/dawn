enable f16;

fn log_8f0e32() {
  var arg_0 = vec2<f16>(1.0h);
  var res : vec2<f16> = log(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  log_8f0e32();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  log_8f0e32();
}

@compute @workgroup_size(1)
fn compute_main() {
  log_8f0e32();
}
