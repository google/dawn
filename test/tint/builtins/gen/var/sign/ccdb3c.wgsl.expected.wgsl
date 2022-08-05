enable f16;

fn sign_ccdb3c() {
  var arg_0 = vec2<f16>(f16());
  var res : vec2<f16> = sign(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sign_ccdb3c();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sign_ccdb3c();
}

@compute @workgroup_size(1)
fn compute_main() {
  sign_ccdb3c();
}
