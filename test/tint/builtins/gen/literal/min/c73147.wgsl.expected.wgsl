fn min_c73147() {
  var res : i32 = min(1i, 1i);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : i32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  min_c73147();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  min_c73147();
}

@compute @workgroup_size(1)
fn compute_main() {
  min_c73147();
}
