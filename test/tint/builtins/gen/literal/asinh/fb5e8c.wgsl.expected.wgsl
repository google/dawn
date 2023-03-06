enable f16;

fn asinh_fb5e8c() {
  var res : vec3<f16> = asinh(vec3<f16>(1.0h));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f16>;

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
