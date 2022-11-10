enable f16;

fn atan_19faea() {
  var res : vec4<f16> = atan(vec4<f16>(1.0h));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  atan_19faea();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  atan_19faea();
}

@compute @workgroup_size(1)
fn compute_main() {
  atan_19faea();
}
