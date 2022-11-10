enable f16;

fn log_8f0e32() {
  var res : vec2<f16> = log(vec2<f16>(1.0h));
}

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
