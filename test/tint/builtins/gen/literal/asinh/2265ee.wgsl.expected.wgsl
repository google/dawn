fn asinh_2265ee() {
  var res : vec3<f32> = asinh(vec3<f32>(1.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  asinh_2265ee();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  asinh_2265ee();
}

@compute @workgroup_size(1)
fn compute_main() {
  asinh_2265ee();
}
