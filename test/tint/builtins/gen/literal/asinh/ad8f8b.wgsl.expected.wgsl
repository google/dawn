enable f16;

fn asinh_ad8f8b() {
  var res : vec2<f16> = asinh(vec2<f16>(1.0h));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  asinh_ad8f8b();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  asinh_ad8f8b();
}

@compute @workgroup_size(1)
fn compute_main() {
  asinh_ad8f8b();
}
