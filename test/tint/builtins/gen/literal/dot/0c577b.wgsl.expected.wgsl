fn dot_0c577b() {
  var res : f32 = dot(vec4<f32>(1.0f), vec4<f32>(1.0f));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  dot_0c577b();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  dot_0c577b();
}

@compute @workgroup_size(1)
fn compute_main() {
  dot_0c577b();
}
