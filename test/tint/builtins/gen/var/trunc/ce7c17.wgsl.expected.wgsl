enable f16;

fn trunc_ce7c17() {
  var arg_0 = vec4<f16>(1.5h);
  var res : vec4<f16> = trunc(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  trunc_ce7c17();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  trunc_ce7c17();
}

@compute @workgroup_size(1)
fn compute_main() {
  trunc_ce7c17();
}
