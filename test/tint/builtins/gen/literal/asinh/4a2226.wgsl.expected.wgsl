fn asinh_4a2226() {
  var res : vec2<f32> = asinh(vec2<f32>(1.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  asinh_4a2226();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  asinh_4a2226();
}

@compute @workgroup_size(1)
fn compute_main() {
  asinh_4a2226();
}
