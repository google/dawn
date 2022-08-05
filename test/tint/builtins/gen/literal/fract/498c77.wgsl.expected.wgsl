enable f16;

fn fract_498c77() {
  var res : vec4<f16> = fract(vec4<f16>(f16()));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  fract_498c77();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  fract_498c77();
}

@compute @workgroup_size(1)
fn compute_main() {
  fract_498c77();
}
