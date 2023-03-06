fn degrees_51f705() {
  var res : f32 = degrees(1.0f);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  degrees_51f705();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  degrees_51f705();
}

@compute @workgroup_size(1)
fn compute_main() {
  degrees_51f705();
}
