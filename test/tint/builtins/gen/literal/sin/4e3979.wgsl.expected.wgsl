fn sin_4e3979() {
  var res : vec4<f32> = sin(vec4<f32>(1.57079637050628662109f));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sin_4e3979();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sin_4e3979();
}

@compute @workgroup_size(1)
fn compute_main() {
  sin_4e3979();
}
