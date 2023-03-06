fn sin_01f241() {
  var res : vec3<f32> = sin(vec3<f32>(1.57079637050628662109f));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sin_01f241();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sin_01f241();
}

@compute @workgroup_size(1)
fn compute_main() {
  sin_01f241();
}
