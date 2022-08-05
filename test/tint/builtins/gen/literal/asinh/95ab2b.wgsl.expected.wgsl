enable f16;

fn asinh_95ab2b() {
  var res : vec4<f16> = asinh(vec4<f16>(f16()));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  asinh_95ab2b();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  asinh_95ab2b();
}

@compute @workgroup_size(1)
fn compute_main() {
  asinh_95ab2b();
}
