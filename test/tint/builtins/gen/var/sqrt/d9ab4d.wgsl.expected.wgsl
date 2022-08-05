enable f16;

fn sqrt_d9ab4d() {
  var arg_0 = vec2<f16>(f16());
  var res : vec2<f16> = sqrt(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sqrt_d9ab4d();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sqrt_d9ab4d();
}

@compute @workgroup_size(1)
fn compute_main() {
  sqrt_d9ab4d();
}
