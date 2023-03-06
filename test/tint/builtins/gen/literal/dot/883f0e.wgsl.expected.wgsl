fn dot_883f0e() {
  var res : f32 = dot(vec2<f32>(1.0f), vec2<f32>(1.0f));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  dot_883f0e();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  dot_883f0e();
}

@compute @workgroup_size(1)
fn compute_main() {
  dot_883f0e();
}
