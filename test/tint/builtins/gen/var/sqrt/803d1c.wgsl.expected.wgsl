enable f16;

fn sqrt_803d1c() {
  var arg_0 = vec4<f16>(f16());
  var res : vec4<f16> = sqrt(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sqrt_803d1c();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sqrt_803d1c();
}

@compute @workgroup_size(1)
fn compute_main() {
  sqrt_803d1c();
}
