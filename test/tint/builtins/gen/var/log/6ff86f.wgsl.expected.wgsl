enable f16;

fn log_6ff86f() {
  var arg_0 = vec3<f16>(f16());
  var res : vec3<f16> = log(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  log_6ff86f();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  log_6ff86f();
}

@compute @workgroup_size(1)
fn compute_main() {
  log_6ff86f();
}
