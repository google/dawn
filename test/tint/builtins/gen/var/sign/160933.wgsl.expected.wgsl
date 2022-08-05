enable f16;

fn sign_160933() {
  var arg_0 = vec4<f16>(f16());
  var res : vec4<f16> = sign(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sign_160933();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sign_160933();
}

@compute @workgroup_size(1)
fn compute_main() {
  sign_160933();
}
