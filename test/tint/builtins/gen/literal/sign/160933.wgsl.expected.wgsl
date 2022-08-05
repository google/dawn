enable f16;

fn sign_160933() {
  var res : vec4<f16> = sign(vec4<f16>(f16()));
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
