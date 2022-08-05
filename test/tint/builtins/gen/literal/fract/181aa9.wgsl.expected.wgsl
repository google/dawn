enable f16;

fn fract_181aa9() {
  var res : vec2<f16> = fract(vec2<f16>(f16()));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  fract_181aa9();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  fract_181aa9();
}

@compute @workgroup_size(1)
fn compute_main() {
  fract_181aa9();
}
