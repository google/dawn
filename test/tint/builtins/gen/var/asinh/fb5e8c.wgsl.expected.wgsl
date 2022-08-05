enable f16;

fn asinh_fb5e8c() {
  var arg_0 = vec3<f16>(f16());
  var res : vec3<f16> = asinh(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  asinh_fb5e8c();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  asinh_fb5e8c();
}

@compute @workgroup_size(1)
fn compute_main() {
  asinh_fb5e8c();
}
