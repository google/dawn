enable f16;

fn atanh_e3b450() {
  var res : vec4<f16> = atanh(vec4<f16>(f16()));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  atanh_e3b450();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  atanh_e3b450();
}

@compute @workgroup_size(1)
fn compute_main() {
  atanh_e3b450();
}
