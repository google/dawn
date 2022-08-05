enable f16;

fn fract_958a1d() {
  var res : vec3<f16> = fract(vec3<f16>(f16()));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  fract_958a1d();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  fract_958a1d();
}

@compute @workgroup_size(1)
fn compute_main() {
  fract_958a1d();
}
