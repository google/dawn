fn transpose_854336() {
  var res : mat3x3<f32> = transpose(mat3x3<f32>(1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : mat3x3<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  transpose_854336();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  transpose_854336();
}

@compute @workgroup_size(1)
fn compute_main() {
  transpose_854336();
}
