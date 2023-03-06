fn floor_66f154() {
  var res : f32 = floor(1.5f);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  floor_66f154();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  floor_66f154();
}

@compute @workgroup_size(1)
fn compute_main() {
  floor_66f154();
}
